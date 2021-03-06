#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <openssl/sha.h>


// from https://github.com/git/git/blob/master/git-compat-util.h

#ifndef FLEX_ARRAY
/*
 * See if our compiler is known to support flexible array members.
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && (!defined(__SUNPRO_C) || (__SUNPRO_C > 0x580))
# define FLEX_ARRAY /* empty */
#elif defined(__GNUC__)
# if (__GNUC__ >= 3)
#  define FLEX_ARRAY /* empty */
# else
#  define FLEX_ARRAY 0 /* older GNU extension */
# endif
#endif

/*
 * Otherwise, default to safer but a bit wasteful traditional style
 */
#ifndef FLEX_ARRAY
# define FLEX_ARRAY 1
#endif
#endif

// from https://github.com/git/git/blob/master/cache.h
#define CACHE_SIGNATURE 0x44495243/* "DIRC" */
struct cache_header {
    uint32_t hdr_signature;
    uint32_t hdr_version;
    uint32_t hdr_entries;
};

struct cache_time {
    uint32_t sec;
    uint32_t nsec;
};

struct stat_data {
    struct cache_time sd_ctime;
    struct cache_time sd_mtime;
    uint32_t sd_dev;
    uint32_t sd_ino;
    uint32_t sd_mode;
    uint32_t sd_uid;
    uint32_t sd_gid;
    uint32_t sd_size;
};

struct cache_entry {
    struct stat_data ce_stat_data;
    uint8_t sha1[20];
    uint16_t ce_namelen;
    char name[FLEX_ARRAY]; /* more */
};


int main() {
    int fd = open(".git/index", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "cannot open file");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st)) {
        fprintf(stderr, "cannot stat file");
        return 1;
    }

    unsigned char *index = (unsigned char*)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    unsigned char *ptr_entry = index + sizeof(struct cache_header);
    struct cache_header *header = (struct cache_header*)index;

    if (header->hdr_signature != htonl(CACHE_SIGNATURE)) {
        fprintf(stderr, "invalid signature");
        return 1;
    }

    uint32_t hdr_entries = ntohl(header->hdr_entries);
    for (uint32_t i = 0; i < hdr_entries; i++) {
        struct cache_entry *entry = (struct cache_entry*)ptr_entry;
        struct stat_data *stat_data = &(entry->ce_stat_data);
        struct stat file_st;
        stat(entry->name, &file_st);
        if (S_ISREG(file_st.st_mode)) {
            stat_data->sd_ctime.sec = htonl(file_st.st_ctim.tv_sec);
            stat_data->sd_ctime.nsec = htonl(file_st.st_ctim.tv_nsec);
            stat_data->sd_mtime.sec = htonl(file_st.st_mtim.tv_sec);
            stat_data->sd_mtime.nsec = htonl(file_st.st_mtim.tv_nsec);
            stat_data->sd_dev = htonl(file_st.st_dev);
            stat_data->sd_ino = htonl(file_st.st_ino);
        }
        ptr_entry += ((unsigned char*)entry->name-ptr_entry+ntohs(entry->ce_namelen)+8)/8*8;
    }

    SHA_CTX c;
    SHA1_Init(&c);
    SHA1_Update(&c, index, st.st_size - 20);
    SHA1_Final(index + st.st_size - 20, &c);

    munmap(index, st.st_size);
    close(fd);

    return 0;
}
