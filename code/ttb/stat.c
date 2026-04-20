
struct stat {
/* 0x000 */    uint64 st_dev;     /* Device.  */
/* 0x008 */    uint64 st_ino;     /* File serial number.  */
/* 0x010 */    uint32 st_mode;    /* File mode.  */
/* 0x014 */    uint32 st_nlink;   /* Link count.  */
/* 0x018 */    uint32 st_uid;     /* User ID of the file's owner.  */
/* 0x01c */    uint32 st_gid;     /* Group ID of the file's group. */
/* 0x020 */    uint64 st_rdev;    /* Device number, if device.  */
/* 0x028 */    uint64 __pad1;
/* 0x030 */     int64 st_size;    /* Size of file, in bytes.  */
/* 0x038 */     int32 st_blksize; /* Optimal block size for I/O.  */
/* 0x03c */     int32 __pad2;
/* 0x040 */     int64 st_blocks;  /* Number 512-byte blocks allocated. */
/* 0x048 */     int32 st_atime;   /* Time of last access.  */
/* 0x04c */    uint32 st_atime_nsec;
/* 0x050 */     int32 st_mtime;   /* Time of last modification.  */
/* 0x054 */    uint32 st_mtime_nsec;
/* 0x058 */     int32 st_ctime;   /* Time of last status change.  */
/* 0x05c */    uint32 st_ctime_nsec;
/* 0x060 */    uint32 __unused4;
/* 0x064 */    uint32 __unused5;
/* sizeof == 0x068 */
}
