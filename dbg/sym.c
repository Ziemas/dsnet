
#include "dsxdb_prototypes.h"
#include "elf.h"

extern unsigned int dot; // defined in mem.c

static LIST_HEAD(syms_list);
static LIST_HEAD(mod_syms_list);
static LIST_HEAD(symlines);

static int add_symline(unsigned int value, char *bp);
static void show_and_free_symline();
static int is_dup_sym(char *bp, char *str);
static int is_gcc_label(char *str);

void clear_module_symbol()
{
    MOD_SYMS *i;

    list_for_each_safe (i, &mod_syms_list, list) {
        list_remove(&i->list);
        ds_free(i);
    }
}

void clear_module_symbol_with_name(char *name)
{
    MOD_SYMS *i;

    list_for_each_safe (i, &mod_syms_list, list) {
        if (strcmp(name, i->name) == 0) {
            list_remove(&i->list);
            ds_free(i);
        }
    }
}

void add_module_symbol(char *name, int adr, int siz, int id)
{
    MOD_SYMS *p;

    p = ds_alloc(strlen(name) + sizeof(*p) + 1);
    if (p) {
        strcpy(p->name, name);
        p->adr = adr;
        p->siz = siz;
        p->id = id;

        list_insert(&mod_syms_list, &p->list);
    }
}

void clear_symbol()
{
    SYMS *sym;

    list_for_each_safe (sym, &syms_list, list) {
        list_remove(&sym->list);

        ds_free(sym->symtab);
        ds_free(sym->shstrtab);
        ds_free(sym->strtab);
        ds_free(sym->shdr);
        ds_free(sym);
    }

    clear_module_symbol();
}

void clear_symbol_with_id(int id)
{
    SYMS *sym;

    list_for_each_safe (sym, &syms_list, list) {
        if (sym->id == id) {
            list_remove(&sym->list);

            ds_free(sym->symtab);
            ds_free(sym->shstrtab);
            ds_free(sym->strtab);
            ds_free(sym->shdr);
            ds_free(sym);
        }
    }
}

int match_symbol(char *str, int nstr, char *name)
{
    if (!str)
        return 1;

    while (*name) {
        if (!ds_strncmp(str, name, nstr))
            return 1;

        ++name;
    }

    return 0;
}

static int add_symline(unsigned int value, char *bp)
{
    SYMLINE *p, *i;
    int n;

    n = strlen(bp) + 1;
    p = ds_alloc(n + sizeof(*p));

    if (!p)
        return -1;

    p->value = value;
    memcpy(i->string, bp, n);

    /* TODO: Verify same sort order */

    list_for_each (i, &symlines, list) {
        if (i->value > value) {
            list_insert(&i->list, &p->list);

            return 0;
        }
    }

    // If we didn't find an insertion point above
    list_insert(&symlines, &p->list);

    return 0;
}

static void show_and_free_symline()
{
    SYMLINE *i;

    list_for_each_safe (i, &symlines, list) {
        list_remove(&i->list);

        ds_printf("%s\n", i->string);
        ds_free(i);
    }
}

int module_base(int id, int base, int shndx, int info)
{
    if (base == -1)
        return 0;

    if (shndx != 0xff1f && (shndx <= 0 || shndx > 0xffef))
        return 0;

    if (base)
        return base;

    return mod_address_by_id(id);
}

int show_symbol(int ac, char **av)
{
    MOD_SYMS *mp;
    char *bp;
    char buf[1024];
    unsigned int value;
    char *str = NULL;
    int nstr;
    int i;
    DS_ELF_SYMTAB *tab;
    SYMS *sym;

    if (ac <= 0)
        return 0;

    ac--;
    av++;

    if (ac > 0 && **av == '-')
        return ds_error("Usage: show symbol [<str>]");

    if (ac > 0) {
        str = *av;
        nstr = strlen(str);

        if (ac > 1)
            return ds_error("Usage: show symbol [<str>]");
    }

    ds_printf(" Value     Size Section  Bind    Type    Name\n");

    list_init(&symlines);

    list_for_each (sym, &syms_list, list) {
        for (i = 0, tab = sym->symtab; i < sym->nsymtab; i++, tab++) {
            if (i == 0) {
                continue;
            }

            if (match_symbol(str, nstr, tab->name ? &sym->strtab[tab->name] : ""))
                break;

            value = tab->value + module_base(sym->id, sym->base, tab->shndx, tab->info);

            bp = buf + ds_sprintf(bp, " %W +%H", value, tab->size);
            switch (tab->shndx) {
                case SHN_HIPROC:
                    bp += ds_sprintf(bp, " RADDR   ");
                    break;
                case SHN_ABS:
                    bp += ds_sprintf(bp, " ABS     ");
                    break;
                case SHN_COMMON:
                    bp += ds_sprintf(bp, " COMMON  ");
                    break;
                case SHN_UNDEF:
                    bp += ds_sprintf(bp, " UNDEF   ");
                    break;
                default:
                    if (tab->shndx && tab->shndx <= sym->shnum) {
                        bp += ds_sprintf(bp, " %-8s", &sym->shstrtab[sym->shdr[tab->shndx].name]);
                    } else {
                        bp += ds_sprintf(bp, "      %H", tab->shndx);
                    }
                    break;
            }

            switch (ELF32_ST_BIND(tab->info)) {
                case STB_LOCAL:
                    bp += ds_sprintf(bp, " local  ");
                    break;
                case STB_GLOBAL:
                    bp += ds_sprintf(bp, " global ");
                    break;
                case STB_WEAK:
                    bp += ds_sprintf(bp, " weak   ");
                    break;
                case STB_LOPROC:
                    bp += ds_sprintf(bp, " loproc ");
                    break;
                case STB_HIPROC:
                    bp += ds_sprintf(bp, " hiproc ");
                    break;
                default:
                    bp += ds_sprintf(bp, " BIND=%d", ELF32_ST_BIND(tab->info));
                    break;
            }

            switch (ELF32_ST_TYPE(tab->info)) {
                case STT_NOTYPE:
                    bp += ds_sprintf(bp, " notype ");
                    break;
                case STT_OBJECT:
                    bp += ds_sprintf(bp, " object ");
                    break;
                case STT_FUNC:
                    bp += ds_sprintf(bp, " func   ");
                    break;
                case STT_SECTION:
                    bp += ds_sprintf(bp, " section");
                    break;
                case STT_FILE:
                    bp += ds_sprintf(bp, " file   ");
                    break;
                case STT_LOPROC:
                    bp += ds_sprintf(bp, " loproc ");
                    break;
                case STT_HIPROC:
                    bp += ds_sprintf(bp, " hiproc ");
                    break;
                default:
                    bp += ds_sprintf(bp, " TYPE=%d", ELF32_ST_TYPE(tab->info));
                    break;
            }

            if (tab->name)
                ds_sprintf(bp, " %s", &sym->strtab[tab->name]);

            if (add_symline(value, buf) < 0)
                break;
        }
    }

    list_for_each (mp, &mod_syms_list, list) {
        if (match_symbol(str, nstr, mp->name)) {
            bp = buf;
            bp += ds_sprintf(bp, " %W +%H", mp->adr, mp->siz);
            bp += ds_sprintf(bp, " Module  ");
            bp += ds_sprintf(bp, " Module  ");
            bp += ds_sprintf(bp, " module ");
            ds_sprintf(bp, " %s", mp->name);

            if (add_symline(value, buf) < 0) {
                break;
            }
        }
    }

    show_and_free_symline();

    return 0;
}

int look_eemod(
    void *stream,
    DS_ELF_EHDR *ehdr,
    DS_ELF_SHDR *shdr,
    int id,
    int base,
    void (*clear_func)(int))
{
    DS_ELF_SHDR *sheemod = NULL;
    EEMOD *eemod = NULL;
    int i;

    if (ehdr->type != 0xFF91)
        return id;

    if (base)
        return id;

    for (i = 0; i < ehdr->shnum; ++i) {
        if (shdr[i].type == 0x70000090)
            sheemod = &shdr[i];
    }

    /* Get id by name */
    if (sheemod && sheemod->size > 0x2b) {
        eemod = ds_fload(stream, 0, sheemod->offset, sheemod->size, 1);
        if (!eemod) {
            goto error;
        }

        if (eemod->moduleinfo == -1) {
            id = mod_id_by_name(id, eemod->modulename, eemod->moduleversion);
            if (id < 1) {
                goto exit;
            }
        }

        clear_func(id);
    }

    if (!id) {
        ds_error("no module name, -b or -id option is needed");
        goto error;
    }

    if (mod_fetch_id(id) < 0) {
        goto error;
    }

exit:
    ds_free(eemod);
    return id;

error:
    ds_free(eemod);
    return -1;
}

int look_iopmod(
    void *stream,
    DS_ELF_EHDR *ehdr,
    DS_ELF_SHDR *shdr,
    int id,
    int base,
    void (*clear_func)(int))
{
    DS_ELF_SHDR *shiopmod = NULL;
    IOPMOD *iopmod = NULL;
    int i;

    if (base)
        return id;

    for (i = 0; i < ehdr->shnum; ++i) {
        if (shdr[i].type == 0x70000080)
            shiopmod = &shdr[i];
    }

    /* Get id by name */
    if (shiopmod && shiopmod->size > 0x1b) {
        iopmod = ds_fload(stream, 0, shiopmod->offset, shiopmod->size, 1);
        if (!iopmod) {
            goto error;
        }

        if (iopmod->moduleinfo == -1) {
            id = mod_id_by_name(id, iopmod->modulename, iopmod->moduleversion);
            if (id <= 0) {
                goto exit;
            }

            clear_func(id);
        }
    }

    if (!id) {
        ds_error("no module name, -b or -id option is needed");
        goto error;
    }

    if (mod_fetch_id(id) < 0) {
        goto error;
    }

exit:
    ds_free(iopmod);
    return id;

error:
    ds_free(iopmod);
    return -1;
}

int load_symbol(void *stream, DS_ELF_EHDR *ehdr, DS_ELF_SHDR *shdr, int symndx, int strndx, int id, int base)
{
    int sym_count;
    SYMS *syms;
    char *strtab = NULL;
    char *shstrtab = NULL;
    DS_ELF_SYMTAB *symtab = NULL;
    DS_ELF_SHDR *shsym = &shdr[symndx];
    int ida;

    if (ehdr->shstrndx) {
        if (ehdr->shnum > ehdr->shstrndx) {
            shstrtab = ds_fload(stream, 0, shdr[ehdr->shstrndx].offset, shdr[ehdr->shstrndx].size, 1);
            if (!shstrtab) {
                goto err;
            }
        }
    }

    sym_count = shsym->size >> 4;
    if (sym_count == 1) {
        ds_printf("Loading no symbols ...\n");
        return 0;
    }

    if (sym_count > 1)
        ds_printf("Loading %d symbols ...\n", sym_count - 1);

    symtab = ds_fload(stream, 0, shsym->offset, sizeof(*symtab), sym_count);
    if (!symtab) {
        goto err;
    }

    strtab = ds_fload(stream, 0, shdr[strndx].offset, 1, shdr[strndx].size);
    if (!strtab) {
        goto err;
    }

#ifdef DSNET_COMPILING_E
    ida = look_eemod(stream, ehdr, shdr, id, base, clear_symbol_with_id);
#elif DSNET_COMPILING_I
    ida = look_iopmod(stream, ehdr, shdr, id, base, clear_symbol_with_id);
#endif /* DSNET_COMPILING_I */

    syms = ds_alloc(sizeof(*syms));
    if (!syms) {
        goto err;
    }

    syms->id = ida;
    syms->base = base;
    syms->symtab = symtab;
    syms->shstrtab = shstrtab;
    syms->strtab = strtab;
    syms->shnum = ehdr->shnum;
    syms->nsymtab = sym_count;

    syms->shdr = ds_alloc(sizeof(DS_ELF_SHDR) * ehdr->shnum);
    if (!syms->shdr) {
        ds_free(syms);
        goto err;
    }

    memcpy(syms->shdr, shdr, sizeof(DS_ELF_SHDR) * ehdr->shnum);

    list_insert(&syms_list, &syms->list);

    return 0;

err:
    ds_free(shstrtab);
    ds_free(symtab);
    ds_free(strtab);
    return -1;
}

static int is_dup_sym(char *bp, char *str)
{
    int n = strlen(str);
    char tmp[1024];
    char *p;

    while (*bp) {
        p = tmp;

        while (*bp && *bp != ',')
            *p++ = *bp++;

        *p = '\0';

        if (!strcmp(tmp, str))
            return 1;

        if (!ds_strncmp(tmp, str, n) && !strcmp("+0x00", &tmp[n]))
            return 1;

        if (*bp == ',')
            ++bp;
    }

    return 0;
}

int address_to_func(unsigned int adr, unsigned int *padr0, unsigned int *padr1)
{
    unsigned int value;
    unsigned int adr1 = 0;
    unsigned int adr0 = 0;
    int i;
    DS_ELF_SYMTAB *p;
    SYMS *syms;


    list_for_each (syms, &syms_list, list) {
        for (i = 0, p = syms->symtab; i < syms->nsymtab; i++, p++) {
            switch (ELF32_ST_TYPE(p->info)) {
                case STT_FUNC:
                case STT_SECTION:
                    value = syms->symtab[i].value + module_base(syms->id, syms->base, p->shndx, p->info);
                    if (!p->size && value != adr) {
                      continue;
                    }

                    if (!adr0 || adr0 > value)
                        adr0 = value;

                    if (!adr1 || adr1 < p->size + value)
                        adr1 = p->size + value;

                    break;
                default:
                    break;
            }
        }
    }

    *padr0 = adr0;
    *padr1 = adr1;

    return 0;
}

int address_to_symbol(char *buf, unsigned int adr, int force_delta)
{
    int v3;               // eax
    int v4;               // eax
    int v6;               // eax
    int v7;               // eax
    MOD_SYMS *mp;         // [esp+0h] [ebp-420h]
    unsigned int value;   // [esp+4h] [ebp-41Ch]
    unsigned int value_1; // [esp+4h] [ebp-41Ch]
    int delta;            // [esp+8h] [ebp-418h]
    int delta_1;          // [esp+8h] [ebp-418h]
    int i;                // [esp+Ch] [ebp-414h]
    char *sp;             // [esp+10h] [ebp-410h]
    char *sp_1;           // [esp+10h] [ebp-410h]
    char str[1024];       // [esp+14h] [ebp-40Ch] BYREF
    char *bp;             // [esp+414h] [ebp-Ch]
    DS_ELF_SYMTAB *p;     // [esp+418h] [ebp-8h]
    SYMS *syms;           // [esp+41Ch] [ebp-4h]

    bp = buf;
    *buf = 0;
    syms = syms_list.head;
LABEL_2:
    if (syms) {
        p = syms->symtab;
        i = 0;
        while (1) {
            if (syms->nsymtab <= i) {
                syms = syms->forw;
                goto LABEL_2;
            }
            switch (p->info & 0xF) {
                case 0:
                case 1:
                case 2:
                    value = p->value;
                    value_1 = module_base(syms->id, syms->base, p->shndx, p->info) + value;
                    delta = 0;
                    if (p->size) {
                        delta = adr - value_1;
                        if (value_1 > adr || delta >= p->size)
                            goto LABEL_20;
                    } else if (value_1 != adr) {
                        goto LABEL_20;
                    }
                    if (!is_gcc_label(&syms->strtab[p->name])) {
                        sp = &str[ds_sprintf(str, "%s", &syms->strtab[p->name])];
                        if (force_delta || delta > 0)
                            ds_sprintf(sp, "+0x%02x", delta);
                        if (!is_dup_sym(buf, str)) {
                            if (bp > buf) {
                                v3 = ds_sprintf(bp, ",");
                                bp += v3;
                            }
                            v4 = ds_sprintf(bp, "%s", str);
                            bp += v4;
                            force_delta = 0;
                        }
                    }
                LABEL_20:
                    ++i;
                    ++p;
                    break;
                default:
                    goto LABEL_20;
            }
        }
    }
    if (bp > buf)
        return bp - buf;
    for (mp = mod_syms_list.head; mp; mp = mp->forw) {
        delta_1 = 0;
        if (mp->siz) {
            delta_1 = adr - mp->adr;
            if (mp->adr > adr || delta_1 >= mp->siz)
                continue;
        } else if (mp->adr != adr) {
            continue;
        }
        sp_1 = &str[ds_sprintf(str, "%s", mp->name)];
        if (force_delta || delta_1 > 0)
            ds_sprintf(sp_1, "+0x%02x", delta_1);
        if (!is_dup_sym(buf, str)) {
            if (bp > buf) {
                v6 = ds_sprintf(bp, ",");
                bp += v6;
            }
            v7 = ds_sprintf(bp, "%s", str);
            bp += v7;
            force_delta = 0;
        }
    }
    return bp - buf;
}

int symbol_to_value(char *name, unsigned int *pv)
{
    int v2;             // eax
    unsigned __int8 v4; // dl
    MOD_SYMS *mp;       // [esp+8h] [ebp-20h]
    int r;              // [esp+Ch] [ebp-1Ch]
    int n;              // [esp+10h] [ebp-18h]
    int i_3;            // [esp+14h] [ebp-14h]
    char *file;         // [esp+18h] [ebp-10h]
    char *cp;           // [esp+1Ch] [ebp-Ch]
    DS_ELF_SYMTAB *p;   // [esp+20h] [ebp-8h]
    SYMS *syms;         // [esp+24h] [ebp-4h]

    if (*name == 58) {
        ++name;
        v2 = 0;
        if (*name > 47 && *name <= 57)
            v2 = 1;
        if (v2)
            return symbol_to_value_by_mdebug(0, name, pv);
    } else {
        for (cp = name; *cp; ++cp) {
            if (*cp == 58) {
                file = (char *)ds_alloc(cp - name + 1);
                if (!file)
                    return -1;
                memcpy(file, name, cp - name);
                file[cp - name] = 0;
                r = symbol_to_value_by_mdebug(file, cp + 1, pv);
                ds_free(file);
                return r;
            }
        }
    }
    if (!strcmp(".", name)) {
        *pv = dot;
        return 0;
    }
    for (syms = syms_list.head; syms; syms = syms->forw) {
        n = 0;
        p = syms->symtab;
        for (i_3 = 0; syms->nsymtab > i_3; ++i_3) {
            v4 = p->info >> 4;
            if (v4 == 1 || v4 <= 1u && !v4) {
                switch (p->info & 0xF) {
                    case 0:
                    case 1:
                    case 2:
                        if (is_gcc_label(&syms->strtab[p->name]) || strcmp(name, &syms->strtab[p->name]))
                            break;
                        *pv = p->value;
                        *pv += module_base(syms->id, syms->base, p->shndx, p->info);
                        if (p->info >> 4)
                            return 0;
                        if (++n != 1)
                            return ds_error("symbol_to_value:non unique local symbol");
                        break;
                    default:
                        break;
                }
            }
            ++p;
        }
        if (n > 0)
            return 0;
    }
    for (mp = mod_syms_list.head; mp; mp = mp->forw) {
        if (!strcmp(name, mp->name)) {
            *pv = mp->adr;
            return 0;
        }
    }
    return -1;
}

int symbol_completion(DS_HISTBUF *hb, char *name)
{
    unsigned __int8 v3;  // dl
    unsigned __int8 v4;  // dl
    MOD_SYMS *mp;        // [esp+0h] [ebp-2Ch]
    MOD_SYMS *mp_1;      // [esp+0h] [ebp-2Ch]
    int col;             // [esp+4h] [ebp-28h]
    int lns;             // [esp+8h] [ebp-24h]
    int lns_1;           // [esp+8h] [ebp-24h]
    int ns;              // [esp+Ch] [ebp-20h]
    int nm;              // [esp+10h] [ebp-1Ch]
    int n;               // [esp+14h] [ebp-18h]
    int i_3;             // [esp+18h] [ebp-14h]
    int i_1;             // [esp+18h] [ebp-14h]
    char *cn;            // [esp+1Ch] [ebp-10h]
    char *p;             // [esp+20h] [ebp-Ch]
    char *p_1;           // [esp+20h] [ebp-Ch]
    char *p_2;           // [esp+20h] [ebp-Ch]
    char *p_3;           // [esp+20h] [ebp-Ch]
    char *p_4;           // [esp+20h] [ebp-Ch]
    char *p_5;           // [esp+20h] [ebp-Ch]
    const char *v22;     // [esp+20h] [ebp-Ch]
    DS_ELF_SYMTAB *st;   // [esp+24h] [ebp-8h]
    DS_ELF_SYMTAB *st_1; // [esp+24h] [ebp-8h]
    SYMS *syms;          // [esp+28h] [ebp-4h]
    SYMS *syms_1;        // [esp+28h] [ebp-4h]

    if (*name == 58)
        ++name;
    for (p = name; *p; ++p) {
        if (*p == 58)
            return 0;
    }
    n = strlen(name);
    nm = 0;
    for (syms = syms_list.head; syms; syms = syms->forw) {
        st = syms->symtab;
        for (i_3 = 0; syms->nsymtab > i_3; ++i_3) {
            v3 = st->info >> 4;
            if (v3 == 1 || v3 <= 1u && !v3) {
                switch (st->info & 0xF) {
                    case 0:
                    case 1:
                    case 2:
                        if (st->name) {
                            if (!is_gcc_label(&syms->strtab[st->name])) {
                                p_1 = &syms->strtab[st->name];
                                if (!ds_strncmp(name, p_1, n)) {
                                    if (++nm == 1) {
                                        cn = p_1;
                                        ns = strlen(p_1);
                                    } else {
                                        lns = ns;
                                        for (ns = 0; lns > ns && p_1[ns] == cn[ns]; ++ns)
                                            ;
                                    }
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            ++st;
        }
    }
    for (mp = mod_syms_list.head; mp; mp = mp->forw) {
        p_2 = mp->name;
        if (!ds_strncmp(name, mp->name, n)) {
            if (++nm == 1) {
                cn = mp->name;
                ns = strlen(p_2);
            } else {
                lns_1 = ns;
                for (ns = 0; lns_1 > ns && p_2[ns] == cn[ns]; ++ns)
                    ;
            }
        }
    }
    if (!nm)
        return 0;
    if (nm == 1) {
        for (p_3 = &cn[n]; *p_3; ++p_3)
            ds_editline(hb, *p_3, 0);
        return 1;
    } else {
        for (p_4 = &cn[n]; *p_4 && p_4 < &cn[ns]; ++p_4)
            ds_editline(hb, *p_4, 0);
        ds_printf("\n");
        col = 0;
        for (syms_1 = syms_list.head; syms_1; syms_1 = syms_1->forw) {
            st_1 = syms_1->symtab;
            for (i_1 = 0; syms_1->nsymtab > i_1; ++i_1) {
                v4 = st_1->info >> 4;
                if (v4 == 1 || v4 <= 1u && !v4) {
                    switch (st_1->info & 0xF) {
                        case 0:
                        case 1:
                        case 2:
                            if (st_1->name) {
                                if (!is_gcc_label(&syms_1->strtab[st_1->name])) {
                                    p_5 = &syms_1->strtab[st_1->name];
                                    if (!ds_strncmp(name, p_5, n)) {
                                        if (col + strlen(p_5) + 1 > 0x4F) {
                                            ds_printf("\n");
                                            col = 0;
                                        }
                                        col += ds_printf(" %s", p_5);
                                    }
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                ++st_1;
            }
        }
        for (mp_1 = mod_syms_list.head; mp_1; mp_1 = mp_1->forw) {
            v22 = mp_1->name;
            if (!ds_strncmp(name, mp_1->name, n)) {
                if (col + strlen(v22) + 1 > 0x4F) {
                    ds_printf("\n");
                    col = 0;
                }
                col += ds_printf(" %s", v22);
            }
        }
        return -1;
    }
}

static int is_gcc_label(char *str)
{
    return !strcmp(str, "gcc2_compiled.") || strcmp(str, "__gnu_compiled_c") == 0;
}
