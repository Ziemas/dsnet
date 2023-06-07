#ifndef DBG_H_
#define DBG_H_

#include "dsnet_prototypes.h"

#ifdef TARGET_EE
#define REG_SIZE 7
#define PRI -48
#define PROTO_TTYP PROTO_E0TTYP
#define PROTO_KTTYP PROTO_EKTTYP
#define PROTO_SDBGP PROTO_ESDBGP
#define PROTO_TDBGP PROTO_ETDBGP
#define PROTO_LOADP PROTO_ELOADP
#define PROTO_DRFP PROTO_DRFP0
#else
#define REG_SIZE 5
#define PRI -64
#define PROTO_TTYP PROTO_I0TTYP
#define PROTO_KTTYP PROTO_IKTTYP
#define PROTO_SDBGP PROTO_ISDBGP
#define PROTO_TDBGP PROTO_ITDBGP
#define PROTO_LOADP PROTO_ILOADP
#define PROTO_DRFP PROTO_DRFP1
#endif

// root/dsnet/dbg/dbg.c

int __cdecl get_handlerlist(DBGP_HDR* phdr);
int __cdecl get_tcb(DBGP_HDR* phdr, int tid);
int __cdecl get_thread_list(DBGP_HDR* phdr);
int __cdecl get_semablock(DBGP_HDR* phdr, int sid);
int __cdecl load_quad_registers(unsigned int* masks, quad* pv, int n);
int __cdecl store_quad_registers(unsigned int* masks, quad* pv, int n);
int __cdecl cont_and_wait_halt(int code, int cnt);
int __cdecl run_and_wait_halt(unsigned int entry_point, unsigned int gp_value, int argc, char* args, int args_len);
int __cdecl break_and_wait_halt();
int __cdecl wait_halt();
int __cdecl get_brkpt(DBGP_BRKPT_DATA* bps, int* pn);
int __cdecl put_brkpt(DBGP_BRKPT_DATA* bps, int n);
int __cdecl is_target_is_running();
int __cdecl send_iload_and_wait(int cmd, int action, unsigned int id, void* ptr, int len, int nodisp);
int __cdecl is_target_exec_ctl();
int __cdecl is_automatic_prefix_breakpoint();
int __cdecl is_describe_ub_all();
int __cdecl is_lstep_stop_at_no_line();
void __cdecl di_format(int* f_adr, int* f_iw, int* f_ba, int* f_ma);
char* __cdecl dr_format_str();
char* __cdecl source_directories_str();
int __cdecl get_help_lang();
char* __cdecl get_help_pager();
void __cdecl dr_default_di();
void __cdecl ex_default_dr();
void __cdecl dr0_default_di();
void __cdecl ex0_default_dr();
void __cdecl dr1_default_di();
void __cdecl ex1_default_dr();
void __cdecl lstep_default_list();
int __cdecl main(int ac, char** av);
int __cdecl IsSupported(int MajorVersion, int MinorVersion);

int __cdecl load_word_registers(unsigned int* masks, unsigned int* pv, int n);
int __cdecl store_word_registers(unsigned int* masks, unsigned int* pv, int n);

// root/dsnet/dbg/mem.c

int __cdecl dmem_cmd(int ac, char** av);
int __cdecl smem_cmd(int ac, char** av);
int __cdecl inp_cmd(int ac, char** av);
int __cdecl out_cmd(int ac, char** av);
int __cdecl bload_cmd(int ac, char** av);
int __cdecl bsave_cmd(int ac, char** av);
int __cdecl pload_cmd(int ac, char** av);

// root/dsnet/dbg/ereg.c

int __cdecl check_reserved_name(char* name);
int __cdecl load_quad_reg(char* name, quad* pv);
int __cdecl store_quad_reg(char* name, quad val);
int __cdecl dreg_cmd(int ac, char** av);
int __cdecl sreg_cmd(int ac, char** av);
int __cdecl hbp_cmd(int ac, char** av);
int __cdecl rload_cmd(int ac, char** av);
int __cdecl rsave_cmd(int ac, char** av);

// root/dsnet/dbg/ireg.c

int __cdecl check_reserved_name(char* name);
int __cdecl load_word_reg(char* name, unsigned int* pv);
int __cdecl store_word_reg(char* name, unsigned int val);
int __cdecl load_quad_reg(char* name, quad* pv);
int __cdecl store_quad_reg(char* name, quad val);
int __cdecl dreg_cmd(int ac, char** av);
int __cdecl sreg_cmd(int ac, char** av);
int __cdecl hbp_cmd(int ac, char** av);
int __cdecl rload_cmd(int ac, char** av);
int __cdecl rsave_cmd(int ac, char** av);

// root/dsnet/dbg/exe.c

int __cdecl store_user_breakpoints();
int __cdecl restore_user_breakpoints();
int __cdecl eval_bp_reg(char* name, unsigned int* padr);
int __cdecl remove_breakpoints();
int __cdecl bp_cmd(int ac, char** av);
int __cdecl ub_cmd(int ac, char** av);
int __cdecl be_cmd(int ac, char** av);
int __cdecl bd_cmd(int ac, char** av);
int __cdecl break_cmd(int ac, char** av);
int __cdecl wait_cmd(int ac, char** av);
void __cdecl display_current_informations(int result);
int __cdecl set_runarg(int ac, char** av);
int __cdecl run_cmd(int ac, char** av);
int __cdecl cont_cmd(int ac, char** av);
int __cdecl until_cmd(int ac, char** av);
int __cdecl step_cmd(int ac, char** av);
int __cdecl next_cmd(int ac, char** av);
int __cdecl lstep_cmd(int ac, char** av);
int __cdecl lnext_cmd(int ac, char** av);
int __cdecl luntil_cmd(int ac, char** av);

// root/dsnet/dbg/opc.c

int __cdecl is_branch_instruction(unsigned int adr, unsigned int ins, unsigned int* ptadr);
int __cdecl di_cmd(int ac, char** av);
int __cdecl as_cmd(int ac, char** av);
int __cdecl eval_bt_reg(char* name, unsigned int* padr);
int __cdecl eval_sfa_reg(char* name, unsigned int* padr);
int __cdecl eval_sfs_reg(char* name, unsigned int* padr);
void __cdecl disp_bt(BT_REG* br, int cnt);
int __cdecl bt_cmd(int ac, char** av);

// root/dsnet/dbg/sym.c

void __cdecl clear_module_symbol();
void __cdecl clear_module_symbol_with_name(char* name);
void __cdecl add_module_symbol(char* name, int adr, int siz, int id);
void __cdecl clear_symbol();
void __cdecl clear_symbol_with_id(int id);
int __cdecl match_symbol(char* str, int nstr, char* name);
int __cdecl module_base(int id, int base, int shndx, int info);
int __cdecl show_symbol(int ac, char** av);
int __cdecl look_iopmod(
        void *stream,
        DS_ELF_EHDR *ehdr,
        DS_ELF_SHDR *shdr,
        int id,
        int base,
        void (__cdecl *clear_func)());
int __cdecl look_eemod(
    void* stream,
    DS_ELF_EHDR* ehdr,
    DS_ELF_SHDR* shdr,
    int id,
    int base,
    void(__cdecl* clear_func)());
int __cdecl load_symbol(void* stream, DS_ELF_EHDR* ehdr, DS_ELF_SHDR* shdr, int symndx, int strndx, int id, int base);
int __cdecl address_to_func(unsigned int adr, unsigned int* padr0, unsigned int* padr1);
int __cdecl address_to_symbol(char* buf, unsigned int adr, int force_delta);
int __cdecl symbol_to_value(char* name, unsigned int* pv);
int __cdecl symbol_completion(DS_HISTBUF* hb, char* name);

// root/dsnet/dbg/mod.c

int __cdecl mod_id_by_name(int id, char* name, int ver);
int __cdecl mod_fetch_id(int id);
unsigned int __cdecl mod_address_by_id(int id);
void __cdecl mod_set_vaddr(int id, unsigned int vaddr);
void __cdecl clear_mod_list();
int __cdecl iload_callback(unsigned int id, int cmd, void* ptr, int len);
int __cdecl mload_cmd(int ac, char** av);
int __cdecl mstart_cmd(int ac, char** av);
int __cdecl mremove_cmd(int ac, char** av);
int __cdecl mlist_cmd(int ac, char** av);
int __cdecl memlist_cmd(int ac, char** av);

// root/dsnet/dbg/vopc.c

int __cdecl vdi_cmd(int ac, char** av);

// root/dsnet/dbg/help.c

int __cdecl dbg_help(char* name);
int __cdecl dbg_help_completion(DS_HISTBUF* hb, char* name);

// root/dsnet/dbg/mdebug.c

void __cdecl clear_mdebug();
void __cdecl clear_mdebug_with_id(int id);
int __cdecl show_mdebug(int ac, char** av);
int __cdecl load_mdebug(
    void* stream,
    DS_ELF_EHDR* elf_header,
    DS_ELF_SHDR* section_header,
    int mindex,
    int id,
    int base,
    char* path);
unsigned int __cdecl file_and_line_to_address(int line, char* path);
char* __cdecl address_to_file_and_line(unsigned int loc, int* pline, int* f_has, int* pdelta, char** ppath);
int __cdecl symbol_to_value_by_mdebug(char* file, char* name, unsigned int* pv);
char* __cdecl string_by_file_and_line(char* fname, int line, char* obj_path);
int __cdecl list_cmd(int ac, char** av);

// root/dsnet/dbg/ethread.c

int __cdecl dt_cmd(int ac, char** av);
void __cdecl disp_tcb(DBGP_HDR* hdr, int fmode);
void __cdecl disp_thread_list_all(DBGP_HDR* hdr);
void __cdecl disp_thread_list(DBGP_EE_THREADLIST_DATA* pdata);
int __cdecl ds_cmd(int ac, char** av);
int __cdecl intr_cmd(int ac, char** av);

#endif // DBG_H_
