#include "dsnet_prototypes.h"

unsigned char ds_sid = 72u;

DSP_BUF *ds_alloc_buf(int proto, int did, void *ptr, int len)
{
    DSP_BUF *db;
    DECI2_HDR *hdr;

    if ((unsigned int)(len + sizeof(DSP_BUF)) > 0xFFFF) {
        ds_error("ds_alloc_buf - too big");
        return 0;
    }

    db = ds_alloc(len + sizeof(DSP_BUF) + sizeof(DECI2_HDR));
    if (!db) {
        return 0;
    }

    list_init(&db->list);
    hdr = (DECI2_HDR *)db->buf;
    hdr->length = len + sizeof(DECI2_HDR);
    hdr->reserved = 0;
    hdr->protocol = proto;
    hdr->source = ds_sid;
    hdr->destination = did;
    if (ptr && len > 0) {
        memcpy(&db->buf[sizeof(DECI2_HDR)], ptr, len);
    }
    return db;
}

DSP_BUF *ds_free_buf(DSP_BUF *db)
{
    return ds_free(db);
}

DSP_BUF *ds_dup_buf(DSP_BUF *db)
{
    int len;
    DSP_BUF *buf;
    DECI2_HDR *hdr;

    hdr = (DECI2_HDR *)db->buf;
    len = hdr->length;
    buf = ds_alloc(len + sizeof(DSP_BUF));
    if (!buf)
        return 0;

    memcpy(buf->buf, db->buf, len);
    list_init(&buf->list);

    return buf;
}
