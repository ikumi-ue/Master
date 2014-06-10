/******************************************************
*   Copyright(C) 2004-2005 RICOH CO.LTD
*   ORDER           : GW_FAXAPL
*   PROGRAM NAME    : FAX
*   FILE NAME       : pref_ctl_defprg.c
*   VERSION         :V1.00
*   DESIGNER        :Osamu Yamasita( G09932 )
*   AUTHOR          :Osamu Yamasita( G09932 )
*------------------------------------------------------
*   HISTORY
*   2010.11.26 �V�K�쐬
*   2012.07.03 �w��L�����R�[�h�d�l�ύX�Ή�(@Yamashita_13S_0003)
*	2014.05.29 �C���|�[�g�E�G�L�X�|�g�[�g�������(@TohruShibuya_15S_0005)
*
*******************************************************/

/* ========================================================================= */
/* �C���N���[�h�t�@�C���錾                                                  */
/* ========================================================================= */
#include <fax_spec.h>

#if FAX_SPEC_PREF

#include <limits.h>
#include "strno.h"
#include "str_uty.h"
#include "fax_nv.h"
#include "ocs_if.h"
#include "fcs_ctl_prg.h"
#include "mode_ctl.h"
#include "fax_adrlib.h"
#include "dat_preview.h"

#include "pref_ctl.h"

/* @TohruShibuya_15S_0005 START */
#include "mode_ctl_cooperate.h"
#include <gw_dcs.h>
/* @TohruShibuya_15S_0005 END */


/* ========================================================================= */
/* �\���̒�`                                                                */
/* ========================================================================= */


/* ========================================================================= */
/* ��������f�[�^��`                                                        */
/* ========================================================================= */
/* �ݒ薼�� */
#ifdef FAXAPL_UWVGA
STRNO PrefStrTbl_defPrg[] = {U_COM_UP00066};		/* ������ʂƂ��ēo�^ */
#else
STRNO PrefStrTbl_defPrg[] = {ANCOM_BT00451};		/* �����l�Ƃ��ēo�^ */
#endif

/* ���M�I�v�V�����F�@�\ON/OFF */
#define TXOPT_OFF					0				/* OFF */
#define TXOPT_ON					1				/* ON */

/* ���M�I�v�V�����F�����~��(0=OFF�A1=�~�ς̂݁A2=�~�ρ{���M) */
#define TXOPT_STROFF				0				/* OFF */
#define TXOPT_STRONLY				1				/* �~�ς̂� */
#define TXOPT_STRTX					2				/* �~��+���M */

/* ���M�I�v�V�����F�t�@�C���`�� */
#define TXOPT_FILETIFF				0				/* TIFF */
#define TXOPT_FILEPDF				1				/* PDF */
#define TXOPT_FILEPDFA				2				/* PDFA */

/* �I�v�V�����������̖������ڂɑ΂��閳���l */ /* @Yamashita_11A12S_0005 */
#define INVALID_VALUE_DEFPRG		9				/* �����l */


/* ========================================================================= */
/* �����֐��v���g�^�C�v�錾                                                  */
/* ========================================================================= */


/* ===================================================================== */
/*                                                                       */
/* ===================================================================== */
/*                                                                       */
/* �֐����́FPrefDefPrgGet                                               */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* �T�@�@�v�F�f�t�H���g�v���O�����擾����                                */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* �p�����^�Fu_char       SecretInfo  �@�����L��                       */
/*           PrefParam_t  *PrefInfo   �v���t�@�����X���                 */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* ���@�@���F�f�t�H���g�v���O�����̃v���t�@�����X�f�[�^�擾����          */
/*                                                                       */
/* ===================================================================== */
u_int PrefDefPrgGet(u_char SecretInfo, PrefParam_t *PrefInfo)
{
    /* ---------------------------------------------- */
    /* �ϐ��錾                                       */
    /* ---------------------------------------------- */
	int						DefNRet,DefSRet;		/* �擾���� */
	short					PrefNameRet;			/* �ݒ薼�̕�����擾���� */
	fax_default_normal_t	DefNInfo,DefSInfo;		/* �f�t�H���g�v���O���� */
	u_char					SysLang = LANG_JP;		/* �V�X�e������ */
	/* @Yamashita_11A12S_0005 START */
	u_char 					IFaxFunc;				/* �C���^�[�l�b�g�t�@�N�X�@�\ *//* @TohruShibuya_15S_0005 */
	u_char 					PcMailFunc;				/* ���[���@�\ *//* @TohruShibuya_15S_0005 */
	u_char 					FolderFunc;				/* �t�H���_�@�\ *//* @TohruShibuya_15S_0005 */
	u_char 					FaxDimm;				/* FCU�g�������� */
	u_char 					DblSideScn;				/* ���ʓǂݎ��@�\ */
	u_char 					Stamp;					/* �σX�^���v�@�\ */
	u_char 					SizeMix;				/* �T�C�Y���ڋ@�\ */
	u_char 					LsFunc;					/* LS�@�\ */
	PV_PREVIEW_FUNC			PreviewEnable;			/* �v���r���[�@�\ */
	int 					SMime;					/* S/MIME�@�\ */
	u_char 					FaxLine;				/* ����L�� */
	gwUsrEnumLine			UcsFaxLine;				/* UCS��`��� */
	u_char					DefFaxLine;				/* Default��� */
	/* @Yamashita_11A12S_0005 END */
	int						StatRet = SUB_STAT_OFF;	/* ��ԃ`�F�b�N���� */ /* @Yamashita_12S_0029 */

/* @TohruShibuya_15S_0005 START */
    u_int					faxCpState = 0;			/* FAX��ԏ�� */
	extern gwDrConfig_t	    dcs_config;				/* �R���t�B�M�����[�V������� */
	u_char					cRetSPDoc;				/* ���ꌴ�e�@�\��� */
	int						iAutoDet;				/* �������m�@�\�L��	*/
/* @TohruShibuya_15S_0005 END */

    /* ---------------------------------------------- */
    /* ����                                           */
    /* ---------------------------------------------- */

	FAX_LOG("PrefDefPrgGet Export START\n");

	if(PrefInfo == NULL){
		return(PREF_RET_SET_ERR);
	}

	/* �f�t�H���g�v���O�������擾(�ʏ���) */
	DefNRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_NORMAL, &DefNInfo);
	FAX_LOG("PrefDefPrgGet:FaxNVDefaultGet[Normal Ret=%d]\n",DefNRet);
	/* �f�t�H���g�v���O�������擾(�ȒP���) */
	DefSRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_SIMPLE, &DefSInfo);
	FAX_LOG("PrefDefPrgGet:FaxNVDefaultGet[Simple Ret=%d]\n",DefSRet);

	/* �擾���ʁF����ȊO */
	if((DefNRet != FAXNV_OK) && (DefSRet != FAXNV_OK)) {
		/* �擾�G���[ */
		FAX_LOG("PrefDefPrgGet FaxNVDefaultGet Error[DefNRet=%d DefSRet=%d]\n", DefNRet, DefSRet);
		return(PREF_RET_SET_ERR);
	}

	/* @Yamashita_12S_0029 START */
	/* OCS READY�`�F�b�N */
	StatRet = ModeFaxStatChk(MAIN_STAT_OCSREADY);
	FAX_LOG("PrefDefPrgGet ModeFaxStatChk[StatRet=%d]\n", StatRet);
	/* OCS READY ON */
	if(StatRet == SUB_STAT_ON){
		/* �v���t�@�����X�ݒ薼�� */
		PrefNameRet = gwOpGetString(PrefInfo->LanguageKind, APLI_ID_FAX, PrefStrTbl_defPrg[0], PrefInfo->prefs_name);
		/* �ݒ薼�̂̕����R�[�h */
		PrefInfo->name_char_code = LangToCharCode(PrefInfo->LanguageKind);
		FAX_LOG("PrefDefPrgGet gwOpGetString[PrefNameRet=%d name_char_code=%d]\n", PrefNameRet, PrefInfo->name_char_code);
		/* ���̎擾NG�̏ꍇ�͎��s�v���Œʒm���ꂽ����ł͂Ȃ������Ŏ��@�̌�����擾���Ė��̂��擾���� */
		if(PrefNameRet != NORMAL_END){
			PrefNameRet = FaxStrOcsIdStrGet(PrefStrTbl_defPrg[0], PrefInfo->prefs_name);
			/* ����w��ŃG�N�X�|�[�g���̂��G���[�ɂ��Ă͂����Ȃ��ƌ����d�l����A�����Ŏ擾�ł��Ȃ��Ă��������s */
			/* �ݒ薼�̂̕����R�[�h */
			SysLang = GetLangType();
			PrefInfo->name_char_code = LangToCharCode(SysLang);
			FAX_LOG("PrefDefPrgGet FaxStrOcsIdStrGet[PrefNameRet=%d name_char_code=%d]\n", PrefNameRet, PrefInfo->name_char_code);
		}
	}
	/* OCS READY OFF */
	else{
		/* PrefInfo->prefs_name �͊���bzero�ς݂̂��߂��̂܂܂ł悢 */
		/* �ݒ薼�̂̕����R�[�h */
		PrefInfo->name_char_code = CHAR_CODE_NONE;
	}
	/* @Yamashita_12S_0029 END */

	/* �ݒ�l�̕����R�[�h */
#if FAX_SPEC_13S /* @Yamashita_13S_0003 */
	PrefInfo->value_char_code = CHAR_CODE_EUSTD;
#else
	PrefInfo->value_char_code = CHAR_CODE_NONE;
#endif
	FAX_LOG("PrefDefPrgGet char_code[name=%d value=%d]\n", PrefInfo->name_char_code, PrefInfo->value_char_code);

	/* @Yamashita_11A12S_0005 START */
	/* ===== �@�\���擾 ===== */
	IFaxFunc = GetFuncIFaxFunc();							/* �C���^�[�l�b�g�t�@�N�X�@�\ *//* @TohruShibuya_15S_0005 */
	PcMailFunc = GetFuncPcMailFunc();						/* ���[���@�\ *//* @TohruShibuya_15S_0005 */
	FolderFunc = GetFuncFolderFunc();						/* �t�H���_�@�\ *//* @TohruShibuya_15S_0005 */
	FaxDimm = GetMachineFaxDIMM();							/* FCU�g�������� */
	DblSideScn = GetFunc2sideScn();							/* ���ʓǂݎ��@�\ */
	Stamp = GetMachineStamp();								/* �σX�^���v�@�\ */
	SizeMix = GetFuncSizeMix();								/* �T�C�Y���ڋ@�\ */
	LsFunc = GetFuncLS();									/* LS�@�\ */
	PreviewEnable = datPV_PreviewEnable();					/* �v���r���[�@�\ */
	SMime = SMimeFuncGet();									/* S/MIME�@�\ */
	UcsFaxLine = CnvLineDtoU(DefNInfo.dial_kind & 0x0f);	/* ����\����`�̉����ʂ�UCS��`�ɕϊ� */
	FaxLine = GetMachineFaxLine(UcsFaxLine);				/* UCS��`����L�� */
	DefFaxLine = FaxLineDefault();							/* Default��� */


/* @TohruShibuya_15S_0005 START */
    faxCpState = ModeCooperateStatGet();                     /* FAX��� */
	cRetSPDoc = GetFuncSPDoc();                             /* ���ꌴ�e�@�\��� */
	iAutoDet = GetMachineAutoDetect();                      /* �������m�@�\�L��	*/
/* @TohruShibuya_15S_0005 END */

	FAX_LOG("PrefDefPrgGet FunInfo[IFaxFunc=%d PcMailFunc=%d FolderFunc=%d FaxDimm=%d DblSideScn=%d Stamp=%d SizeMix=%d LsFunc=%d PreviewEnable=%d SMime=%d FaxLine=%d faxCpState =%d cRetSPDoc =%d iAutoDet = %d]\n",
			IFaxFunc, PcMailFunc, FolderFunc, FaxDimm, DblSideScn, Stamp, SizeMix, LsFunc, PreviewEnable, SMime, FaxLine,faxCpState,cRetSPDoc,iAutoDet);
	/* @Yamashita_11A12S_0005 END */

	/* ===== �v���t�@�����X�ݒ� ===== */
	/* <<<<< �ʏ��� >>>>> */


/* @TohruShibuya_15S_0005 START */
	/* ------------------------ */
	/* �W���u���[�h             */
	/* 0=���ڑ��M�A1=���������M */
	/* ------------------------ */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if(faxCpState & FAXCP_STAT_CLIENT){
		/* ���������M�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[0], "%d", FAXNV_DEF_JOBMODE_MEM);
	}else{
		(void)sprintf(&PrefInfo->prefs_value[0], "%d", DefNInfo.job_mode);
	}

	/* ---------------------------------------------------------------------- */
	/* ��^���I��ԍ�                                                         */
	/* 0=��^�������A1=�y�}����z�A2=�y���}�z�A3=�y�֌W��ɔz�z���Ă��������z */
	/* 4=�y�d�b���������z�A5=�o�^��1�A6=�o�^��2�A7=�o�^��3                    */
	/* ---------------------------------------------------------------------- */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if(faxCpState & FAXCP_STAT_CLIENT){
		/* ��^���Ȃ� */
		(void)sprintf(&PrefInfo->prefs_value[1], "%d",FAXNV_DEF_FORM_NONE);
	}else{
		(void)sprintf(&PrefInfo->prefs_value[1], "%d", DefNInfo.string_num);
	}

	/* ---------------------------------------------------------------------- */
	/* ���[���{��                                                             */
	/* 0=�{�������A1=�{��No.1�A2=�{��No.2�A3=�{��No.3�A4=�{��No.4�A5=�{��No.5 */
	/* ---------------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [�C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� ���� ���[���@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@]�܂���[HDD�Ȃ�] */
	if(  ((IFaxFunc == DAT_OFF)&&(PcMailFunc == DAT_OFF)) ||
		 (faxCpState & FAXCP_STAT_CLIENT) || (dcs_config.max_typical_mail_body <= 0)){
		 /* �{������ */
		(void)sprintf(&PrefInfo->prefs_value[2], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[2], "%d", DefNInfo.mail_id);
	}

	/* -------------------------------------------- */
	/* �����T�C�Y(0=�ӂ����A1=�����Ȏ��A2=���׎�) */
	/* -------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [FCU�g���������Ȃ�]����[���׎��I��]�� */
	if( (FaxDimm == DAT_OFF) && (DefNInfo.scan_set.str_size == FAXNV_DEF_SCAN_RESOL_SFINE) ){
		/* �ӂ��� */
		(void)sprintf(&PrefInfo->prefs_value[3], "%d", FAXNV_DEF_SCAN_RESOL_STD);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[3], "%d", DefNInfo.scan_set.str_size);
	}

	/* ---------------------------------------------------------------------------- */
	/* ���e���                                                                     */
	/* 0=�����A1=�����E�}�\�A2=�����E�ʐ^�A3=�ʐ^�A4=���[�U�[���e1�A5=���[�U�[���e2 */
	/* ---------------------------------------------------------------------------- */

	/* [���ꌴ�e�@�\OFF]����[���[�U�[���e�P�A�Q]�I���� */
	if((cRetSPDoc == DAT_OFF) && ((DefNInfo.scan_set.doc_kind == FAXNV_DEF_SCAN_ORGNL_USER1) ||
		(DefNInfo.scan_set.doc_kind == FAXNV_DEF_SCAN_ORGNL_USER2))  ){
		/* �����Œ� */
		(void)sprintf(&PrefInfo->prefs_value[4], "%d", FAXNV_DEF_SCAN_ORGNL_TXT);
	}else{
		(void)sprintf(&PrefInfo->prefs_value[4], "%d", DefNInfo.scan_set.doc_kind);
	}

	/* ---------------- */
	/* �蓮�Z�x�l(0�`7) */
	/* ---------------- */
	(void)sprintf(&PrefInfo->prefs_value[5], "%d", DefNInfo.scan_set.notch);

	/* --------------------- */
	/* �����Z�x(0=OFF�A1=ON) */
	/* --------------------- */
	(void)sprintf(&PrefInfo->prefs_value[6], "%d", DefNInfo.scan_set.auto_conc);

	/* --------------------------------------------------- */
	/* �Ж�/����(0=�ЖʁA1=1���ڂ��痼�ʁA2=2���ڂ��痼��) */
	/* --------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [���ʓǂݎ��@�\]�Ȃ� */
	if(DblSideScn == DAT_OFF){
		/* �ЖʌŒ� */
		(void)sprintf(&PrefInfo->prefs_value[7], "%d", FAXNV_DEF_SCAN_DBL_NONE);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[7], "%d", DefNInfo.scan_set.dbl_scan.func);
	}

	/* -------------------------------- */
	/* �J������(0=���E�J���A1=�㉺�J��) */
	/* -------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [���ʓǂݎ��@�\]�Ȃ� */
	if(DblSideScn == DAT_OFF){
		/* ���E�J�� */
		(void)sprintf(&PrefInfo->prefs_value[8], "%d", FAXNV_DEF_SCAN_ORGOPEN_LR);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[8], "%d", DefNInfo.scan_set.dbl_scan.org_open);
	}

	/* ----------------------- */
	/* �σX�^���v(0=OFF�A1=ON) */
	/* ----------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [�σX�^���v�@�\]�Ȃ� */
	if(Stamp == DAT_OFF){
		/* �σX�^���vOFF */
		(void)sprintf(&PrefInfo->prefs_value[9], "%d", FAXNV_DEF_SCAN_STAMP_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[9], "%d", DefNInfo.scan_set.set_stamp);
	}

	/* -------------------------------------------- */
	/* ���e�Z�b�g����(0=�ǂ߂�����A1=�ǂ߂Ȃ�����) */
	/* -------------------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[10], "%d", DefNInfo.scan_set.org_set);

	/* ------------------------------------------------------------------------ */
	/* �ǂݎ��T�C�Y�ݒ���                                                   */
	/* 0=�������m�A1=��`�T�C�Y�A2=�o�^�T�C�Y1�A3=�o�^�T�C�Y2�A4=���e�T�C�Y���� */
	/* ------------------------------------------------------------------------ */
	/* @Yamashita_11A12S_0005 */
	/* [�������m�@�\]�Ȃ� ���� [�������m] */
	if((iAutoDet == DAT_OFF)&& (DefNInfo.scan_set.org_size.type == FAXNV_SCAN_SIZE_AUTO)){
		/* ��`�T�C�Y */
		(void)sprintf(&PrefInfo->prefs_value[11], "%d", FAXNV_SCAN_SIZE_STD);
	}else{
		/* [�T�C�Y���ڋ@�\]�Ȃ� ���� [�T�C�Y����] */
		if((SizeMix == DAT_OFF) && (DefNInfo.scan_set.org_size.type == FAXNV_SCAN_SIZE_MIX)){
			/* �������m */
			(void)sprintf(&PrefInfo->prefs_value[11], "%d", FAXNV_SCAN_SIZE_AUTO);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[11], "%d", DefNInfo.scan_set.org_size.type);
		}
	}

	/* --------------------------------------------------------------------------------------------- */
	/* �ǂݎ��T�C�Y��`�T�C�Y(�ő�2���̐���������)                                                 */
	/* 0=A4���R�A1=A4�^�e�A2=B4�^�e�A3=A3�^�e�A4=LT���R�A5=LT�^�e�A6=LG�^�e�A7=DLT�^�e               */
	/* 8=B4���R�A9=A3���R�A10=B3�^�e�A11=A2�^�e�A12=LG���R�A13=DLT���R�A14=17�~22�^�e�A15=A3�m�r�^�e */
	/* --------------------------------------------------------------------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[12], "%2d", DefNInfo.scan_set.org_size.size);

	/* ----------------------------------------------------------------- */
	/* ��T�C�Y���e�w��(0=OFF�A1=1���ڂ����T�C�Y�A2=2���ڂ����T�C�Y) */
	/* ----------------------------------------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[14], "%d", DefNInfo.scan_set.large_doc.func);

	/* ---------------------------------- */
	/* ��T�C�Y���e�w��T�C�Y(0=A2�A1=B3) */
	/* ---------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[15], "%d", DefNInfo.scan_set.large_doc.org_size);

	/* ### ���M�I�v�V���� ### */
	/* --------------------------- */
	/* Bcc���M(0=OFF�A1=ON)        */
	/* FAXNV_DEF_TXOPT_BCCTX(1<<0) */
	/* --------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [�C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� ���� ���[���@�\�Ȃ�]�܂��� [�t�@�N�X�I�v�V���������ڋ@]*/
	if( ((IFaxFunc == DAT_OFF) && (PcMailFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[16], "%d", TXOPT_OFF);
	}
	else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_BCCTX) == FAXNV_DEF_TXOPT_BCCTX){
			(void)sprintf(&PrefInfo->prefs_value[16], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[16], "%d", TXOPT_OFF);
		}
	}
	/* ----------------------------- */
	/* ���M���ʃ��|�[�g(0=OFF�A1=ON) */
	/* FAXNV_DEF_TXOPT_REPORT(1<<1)  */
	/* ----------------------------- */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if(faxCpState & FAXCP_STAT_CLIENT){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[17], "%d", TXOPT_OFF);
	}else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_REPORT) == FAXNV_DEF_TXOPT_REPORT){
			(void)sprintf(&PrefInfo->prefs_value[17], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[17], "%d", TXOPT_OFF);
		}
	}
	/* ------------------------------- */
	/* ��M�m�F(0=OFF�A1=ON)           */
	/* FAXNV_DEF_TXOPT_CHKRECEPT(1<<2) */
	/* ------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [�C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� ���� ���[���@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@] */
	if(((IFaxFunc == DAT_OFF) && (PcMailFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[18], "%d", TXOPT_OFF);
	}
	else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_CHKRECEPT) == FAXNV_DEF_TXOPT_CHKRECEPT){
			(void)sprintf(&PrefInfo->prefs_value[18], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[18], "%d", TXOPT_OFF);
		}
	}
	/* -------------------------------- */
	/* �����k��(0=OFF�A1=ON)            */
	/* FAXNV_DEF_TXOPT_AUTOREDUCE(1<<3) */
	/* -------------------------------- */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* ON�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[19], "%d", TXOPT_ON);
	}else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_AUTOREDUCE) == FAXNV_DEF_TXOPT_AUTOREDUCE){
			(void)sprintf(&PrefInfo->prefs_value[19], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[19], "%d", TXOPT_OFF);
		}
	}

	/* -------------------------- */
	/* ID���M(0=OFF�A1=ON)        */
	/* FAXNV_DEF_TXOPT_IDTX(1<<4) */
	/* -------------------------- */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[20], "%d", TXOPT_OFF);
	}else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_IDTX) == FAXNV_DEF_TXOPT_IDTX){
			(void)sprintf(&PrefInfo->prefs_value[20], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[20], "%d", TXOPT_OFF);
		}
	}
	/* ----------------------------------------- */
	/* �����~��(0=OFF�A1=�~�ς̂݁A2=�~�ρ{���M) */
	/* FAXNV_DEF_TXOPT_STRONLY(1<<5)�~�ς̂�     */
	/* FAXNV_DEF_TXOPT_STRTX  (1<<6)�~��+���M    */
	/* ----------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [LS�@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@] */
	if((LsFunc == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[21], "%d", TXOPT_STROFF);
	}
	else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_STRONLY) == FAXNV_DEF_TXOPT_STRONLY){
			(void)sprintf(&PrefInfo->prefs_value[21], "%d", TXOPT_STRONLY);
		}
		else if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_STRTX) == FAXNV_DEF_TXOPT_STRTX){
			(void)sprintf(&PrefInfo->prefs_value[21], "%d", TXOPT_STRTX);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[21], "%d", TXOPT_STROFF);
		}
	}
	/* ----------------------------- */
	/* F�R�[�h���M(0=OFF�A1=ON)      */
	/* FAXNV_DEF_TXOPT_FCODETX(1<<7) */
	/* ----------------------------- */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[22], "%d", TXOPT_OFF);
	}else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_FCODETX) == FAXNV_DEF_TXOPT_FCODETX){
			(void)sprintf(&PrefInfo->prefs_value[22], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[22], "%d", TXOPT_OFF);
		}
	}
	/* ----------------------------- */
	/* F�R�[�h���o��(0=OFF�A1=ON)  */
	/* FAXNV_DEF_TXOPT_FCODERX(1<<8) */
	/* ----------------------------- */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[23], "%d", TXOPT_OFF);
	}else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_FCODERX) == FAXNV_DEF_TXOPT_FCODERX){
			(void)sprintf(&PrefInfo->prefs_value[23], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[23], "%d", TXOPT_OFF);
		}
	}

	/* ------------------------------ */
	/* ������������(0=OFF�A1=ON)      */
	/* FAXNV_DEF_TXOPT_LABELINS(1<<9) */
	/* ------------------------------ */
	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[24], "%d", TXOPT_OFF);
	}else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_LABELINS) == FAXNV_DEF_TXOPT_LABELINS){
			(void)sprintf(&PrefInfo->prefs_value[24], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[24], "%d", TXOPT_OFF);
		}
	}

	/* --------------------------- */
	/* ����(0=OFF�A1=ON)           */
	/* FAXNV_DEF_TXOPT_SIGN(1<<10) */
	/* --------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [S/MIME�@�\�Ȃ�] �܂��� [���[���@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@]  */
	if((SMime == DAT_OFF) || (PcMailFunc == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT) ){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[25], "%d", TXOPT_OFF);
	}
	else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_SIGN) == FAXNV_DEF_TXOPT_SIGN){
			(void)sprintf(&PrefInfo->prefs_value[25], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[25], "%d", TXOPT_OFF);
		}
	}

	/* --------------------------------- */
	/* �Í���(0=OFF�A1=ON)               */
	/* FAXNV_DEF_TXOPT_ENCRYPTION(1<<11) */
	/* --------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [S/MIME�@�\�Ȃ�] �܂��� [���[���@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@]  */
	if((SMime == DAT_OFF) || (PcMailFunc == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT) ){
		(void)sprintf(&PrefInfo->prefs_value[26], "%d", TXOPT_OFF);
	}
	else{
		if((DefNInfo.tx_option & FAXNV_DEF_TXOPT_ENCRYPTION) == FAXNV_DEF_TXOPT_ENCRYPTION){
			(void)sprintf(&PrefInfo->prefs_value[26], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[26], "%d", TXOPT_OFF);
		}
	}


	/* ------------------------- */
	/* �����w�著�M              */
	/* ------------------------- */

	/* �t�@�N�X�I�v�V���������ڋ@�̏ꍇ */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* ------------------------- */
		/* �����w�著�M(0=OFF�A1=ON) */
		/* ------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[27], "%d", TXOPT_OFF);

		/* ----------------------------------------------------------- */
		/* �����w�著�M����(24�����ŕ\����4���̐���������A0000�`2359) */
		/* ----------------------------------------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[28], "%02d", 0);
		(void)sprintf(&PrefInfo->prefs_value[30], "%02d", 0);
	}else{
		/* ------------------------- */
		/* �����w�著�M(0=OFF�A1=ON) */
		/* ------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[27], "%d", DefNInfo.time_later.func);

		/* ----------------------------------------------------------- */
		/* �����w�著�M����(24�����ŕ\����4���̐���������A0000�`2359) */
		/* ----------------------------------------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[28], "%02d", DefNInfo.time_later.hour);
		(void)sprintf(&PrefInfo->prefs_value[30], "%02d", DefNInfo.time_later.min);
	}

	/* ------------------------------------------------------------------------------------------ */
	/* ���M�����̈�(�ő�2���̐���������)                                                        */
	/* 0=OFF�A1=��1���M�����́A2=��2���M�����́A3=��3���M�����́A4=��4���M�����́A5=��5���M������ */
	/* 6=��6���M�����́A7=��7���M�����́A8=��8���M�����́A9=��9���M�����́A10=��10���M������      */
	/* ------------------------------------------------------------------------------------------ */
	(void)sprintf(&PrefInfo->prefs_value[32], "%2d", DefNInfo.send_agen);

	/* -------------------------------------------------------------------------- */
	/* FAX������(�ő�2���̐���������):����\����`                              */
	/* 1=G3�A2=G3�����A3=G3-1�A4=G3-1�����A5=G3-2�A6=G3-2�����A7=G3-3�A8=G3-3���� */
	/* 9=G3��A10=G3������A11=I-G3�A12=I-G3�����A13=G4�A14=H323�A15=SIP          */
	/* -------------------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* ����Ȃ� */
	if(FaxLine == DAT_OFF){
		(void)sprintf(&PrefInfo->prefs_value[34], "%2d", DefFaxLine); /* Default��� */
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[34], "%2d", (DefNInfo.dial_kind & 0x0f));
	}

	/* -------------------------------------------------------------------------------- */
	/* �C���^�[�l�b�g�t�@�N�XSMTP�I��(0=SMTP�T�[�o�[�o�R����A1=SMTP�T�[�o�[�o�R���Ȃ�) */
	/* -------------------------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* �C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� */
	if(IFaxFunc == DAT_OFF){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[36], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[36], "%d", (((DefNInfo.dial_kind) &(1 << IFAX_SMTP)) >> IFAX_SMTP));
	}

	/* ---------------------------------------------------------------- */
	/* ���[��SMTP�I��(0=SMTP�T�[�o�[�o�R����A1=SMTP�T�[�o�[�o�R���Ȃ�) */
	/* ---------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* ���[���@�\�Ȃ� */
	if(PcMailFunc == DAT_OFF){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[37], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[37], "%d", (((DefNInfo.dial_kind) & (1 << PCMAIL_SMTP)) >> PCMAIL_SMTP));
	}

	/* --------------------------------------------------------------- */
	/* ������(0=FAX�A1=�C���^�[�l�b�g�t�@�N�X�A2=���[���A3=�t�H���_) */
	/* --------------------------------------------------------------- */
	 /* @Yamashita_11A12S_0005 */
	/* �C���^�[�l�b�g�t�@�N�X ���� �C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� */
	if((DefNInfo.dial_mode == DIALMODE_IFAX) && (IFaxFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DIALMODE_FAX);
	}
	/* ���[�� ���� ���[���@�\�Ȃ� */
	else if((DefNInfo.dial_mode == DIALMODE_PCMAIL) && (PcMailFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DIALMODE_FAX);
	}
	/* �t�H���_ ���� �t�H���_�@�\�Ȃ� */
	else if((DefNInfo.dial_mode == DIALMODE_FOLDER) && (FolderFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DIALMODE_FAX);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DefNInfo.dial_mode);
	}

	/* ----------------------------- */
	/* ���M�O�v���r���[(0=OFF�A1=ON) */
	/* ----------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [�v���r���[�@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@] */
	if((PreviewEnable == ePV_PREVIEW_FUNC_OFF) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[39], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[39], "%d", DefNInfo.tx_preview);
	}

	/* ------------------------------------ */
	/* �t�@�C���`��(0=TIFF�A1=PDF�A2=PDF/A) */
	/* ------------------------------------ */
	/* @Yamashita_11A12S_0005 */
	/* [���[���@�\�Ȃ� ���� �t�H���_�@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@] */
	if( ((PcMailFunc == DAT_OFF) && (FolderFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* TIFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[40], "%d", TXOPT_FILETIFF);
	}
	else{
		/* @Yamashita_11A12S_0008 START */
		/* PDF */
		if((DefNInfo.scan_set.tx_fileopt & FAXNV_DEF_TXOPT_FILEPDF) == FAXNV_DEF_TXOPT_FILEPDF){
			(void)sprintf(&PrefInfo->prefs_value[40], "%d", TXOPT_FILEPDF);
		}
		/* PDFA */
		else if((DefNInfo.scan_set.tx_fileopt & FAXNV_DEF_TXOPT_FILEPDFA) == FAXNV_DEF_TXOPT_FILEPDFA){
			(void)sprintf(&PrefInfo->prefs_value[40], "%d", TXOPT_FILEPDFA);
		}
		/* TIFF */
		else{
			(void)sprintf(&PrefInfo->prefs_value[40], "%d", TXOPT_FILETIFF);
		}
		/* @Yamashita_11A12S_0008 END */
	}


	/* ---------------------------- */
	/* �f�W�^������PDF(0=OFF�A1=ON) */
	/* ---------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [���[���@�\�Ȃ� ���� �t�H���_�@�\�Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@] */
	if(((PcMailFunc == DAT_OFF) && (FolderFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF�Œ� */
		(void)sprintf(&PrefInfo->prefs_value[41], "%d", TXOPT_OFF);
	}
	else{
		if((DefNInfo.scan_set.tx_fileopt & FAXNV_DEF_TXOPT_FILESIGN) == FAXNV_DEF_TXOPT_FILESIGN){
			(void)sprintf(&PrefInfo->prefs_value[41], "%d", TXOPT_ON);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[41], "%d", TXOPT_OFF);
		}
	}

	/* <<<<< �ȒP��� >>>>> */
	/* -------------------------------------------- */
	/* �����T�C�Y(0=�ӂ����A1=�����Ȏ��A2=���׎�) */
	/* -------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [FCU�g���������Ȃ�]�܂���[�t�@�N�X�I�v�V���������ڋ@]�����׎��I���� */
	if(  ((FaxDimm == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (DefSInfo.scan_set.str_size == FAXNV_DEF_SCAN_RESOL_SFINE)){
		/* �ӂ��� */
		(void)sprintf(&PrefInfo->prefs_value[42], "%d", FAXNV_DEF_SCAN_RESOL_STD);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[42], "%d", DefSInfo.scan_set.str_size);
	}

	/* --------------------------------------------------- */
	/* �Ж�/����(0=�ЖʁA1=1���ڂ��痼�ʁA2=2���ڂ��痼��) */
	/* --------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* ���ʓǂݎ��@�\�Ȃ� */
	if(DblSideScn == DAT_OFF){
		 /* �Ж� */
		(void)sprintf(&PrefInfo->prefs_value[43], "%d", FAXNV_DEF_SCAN_DBL_NONE);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[43], "%d", DefSInfo.scan_set.dbl_scan.func);
	}
/* @TohruShibuya_15S_0005 END */

	/* --------------------------------------------------------------- */
	/* ������(0=FAX�A1=�C���^�[�l�b�g�t�@�N�X�A2=���[���A3=�t�H���_) */
	/* --------------------------------------------------------------- */
	 /* @Yamashita_11A12S_0005 */
	/* �C���^�[�l�b�g�t�@�N�X ���� �C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� */
	if((DefNInfo.dial_mode == DIALMODE_IFAX) && (IFaxFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DIALMODE_FAX);
	}
	/* ���[�� ���� ���[���@�\�Ȃ� */
	else if((DefNInfo.dial_mode == DIALMODE_PCMAIL) && (PcMailFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DIALMODE_FAX);
	}
	/* �t�H���_ ���� �t�H���_�@�\�Ȃ� */
	else if((DefNInfo.dial_mode == DIALMODE_FOLDER) && (FolderFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DIALMODE_FAX);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DefSInfo.dial_mode);
	}

	/* �ݒ萬�� */
	FAX_LOG("PrefDefPrgGet N[job_mode=%d string_num=%d mail_id=%d str_size=%d doc_kind=%d notch=%d auto_conc=%d dbl_scan.func=%d]\n",
			DefNInfo.job_mode, DefNInfo.string_num, DefNInfo.mail_id, DefNInfo.scan_set.str_size, DefNInfo.scan_set.doc_kind,
			DefNInfo.scan_set.notch, DefNInfo.scan_set.auto_conc, DefNInfo.scan_set.dbl_scan.func);
	FAX_LOG("PrefDefPrgGet N[org_open=%d set_stamp=%d org_set=%d org_size.type=%d org_size.size=%d large_doc.func=%d large_doc.org_size=%d tx_option=0x%04x]\n",
			DefNInfo.scan_set.dbl_scan.org_open, DefNInfo.scan_set.set_stamp, DefNInfo.scan_set.org_set, DefNInfo.scan_set.org_size.type,
			DefNInfo.scan_set.org_size.size, DefNInfo.scan_set.large_doc.func, DefNInfo.scan_set.large_doc.org_size, DefNInfo.tx_option);
	FAX_LOG("PrefDefPrgGet N[time_later.func=%d time_later.hour=%d time_later.min=%d send_agen=%d dial_kind=0x%02x dial_mode=%d tx_preview=%d tx_fileopt=%d]\n",
			DefNInfo.time_later.func, DefNInfo.time_later.hour, DefNInfo.time_later.min, DefNInfo.send_agen, DefNInfo.dial_kind,
			DefNInfo.dial_mode, DefNInfo.tx_preview, DefNInfo.scan_set.tx_fileopt);
	FAX_LOG("PrefDefPrgGet S[str_size=%d dbl_scan.func=%d dial_mode=%d]\n",
			DefSInfo.scan_set.str_size, DefSInfo.scan_set.dbl_scan.func, DefSInfo.dial_mode);

	return(PREF_RET_OK);

}

/* ===================================================================== */
/*                                                                       */
/* �֐����́FPrefDefPrgSet                                               */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* �T�@�@�v�F�f�t�H���g�v���O�����ݒ菈��                                */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* �p�����^�Fu_char       SecretInfo  �@�����L��                       */
/*           PrefParam_t  *PrefInfo   �v���t�@�����X���                 */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* ���@�@���F�f�t�H���g�v���O�����̃v���t�@�����X�f�[�^�ݒ菈��          */
/*                                                                       */
/* ===================================================================== */
u_int PrefDefPrgSet(u_char SecretInfo, PrefParam_t *PrefInfo)
{
    /* ---------------------------------------------- */
    /* �ϐ��錾                                       */
    /* ---------------------------------------------- */
	int						DefNRet,DefSRet;		/* �擾���� */
	u_int					Ret;					/* ���s���� */
	u_int					ChkRet;					/* �`�F�b�N���� */
	fax_default_normal_t	DefNInfo,DefSInfo;		/* �f�t�H���g�v���O���� */
	u_short					TxOptData = TXOPT_ON;	/* ���M�I�v�V�����f�[�^ */
	u_char					DialKindSmtp = 0;		/* SMTP�I����� */
	u_char					TxFileOpt = 0;			/* �t�@�C���`�� */
	u_char					WkStr[3];
	u_char					WkNumC;
	u_short					WkNumS;
	char					*s;
	/* @Yamashita_11A12S_0005 START */
	u_char 					IFaxEnable;				/* �C���^�[�l�b�g�t�@�N�X�@�\ */
	u_char 					PcMailEnable;			/* ���[���@�\ */
	u_char 					FolderEnable;			/* �t�H���_�@�\ */
	u_char 					FaxDimm;				/* FCU�g�������� */
	u_char 					DblSideScn;				/* ���ʓǂݎ��@�\ */
	u_char 					Stamp;					/* �σX�^���v�@�\ */
	u_char 					SizeMix;				/* �T�C�Y���ڋ@�\ */
	u_char 					LsFunc;					/* LS�@�\ */
	PV_PREVIEW_FUNC			PreviewEnable;			/* �v���r���[�@�\ */
	int 					SMime;					/* S/MIME�@�\ */
	u_char 					FaxLine;				/* ����L�� */
	gwUsrEnumLine			UcsFaxLine;				/* UCS��`��� */
	/* @Yamashita_11A12S_0005 END */


/* @TohruShibuya_15S_0005 START */
    u_int					faxCpState = 0;			/* FAX��ԏ�� */
	extern gwDrConfig_t		dcs_config;				/* �R���t�B�M�����[�V������� */
	u_char					cRetSPDoc;				/* ���ꌴ�e�@�\��� */
	int						iAutoDet;				/* �������m�@�\�L�� */
    int						a4model;				/* A4�@�픻�f */

    /* ---------------------------------------------- */
    /* ����                                           */
    /* ---------------------------------------------- */

	FAX_LOG("PrefDefPrgSet Import START\n");

	if(PrefInfo == NULL){
		return(PREF_RET_SET_ERR);
	}

	ChkRet = PREF_RET_OK;

	/* @Yamashita_11A12S_0005 START */
	/* -------------------------------------------------------------------- */
	/* �I�v�V�����L���@�Ԃł̃C���|�[�g�Ή��̂��߁A��[���@�̏����擾��   */
	/* ���̏��ɑ΂��ď������肵�C���|�[�g�f�[�^���㏑�����Ă������@�Ƃ��� */
	/* -------------------------------------------------------------------- */
	/* �f�t�H���g�v���O�������擾(�ʏ���) */
	DefNRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_NORMAL, &DefNInfo);
	FAX_LOG("PrefDefPrgSet:FaxNVDefaultGet[Normal Ret=%d]\n",DefNRet);
	/* �f�t�H���g�v���O�������擾(�ȒP���) */
	DefSRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_SIMPLE, &DefSInfo);
	FAX_LOG("PrefDefPrgSet:FaxNVDefaultGet[Simple Ret=%d]\n",DefSRet);
	/* �擾���� */
	if((DefNRet != FAXNV_OK) && (DefSRet != FAXNV_OK)) {
		/* �擾�G���[ */
		FAX_LOG("PrefDefPrgSet FaxNVDefaultGet Error[DefNRet=%d DefSRet=%d]\n", DefNRet, DefSRet);
		return(PREF_RET_SET_ERR);
	}
	/* ===== �@�\���擾 ===== */
	IFaxEnable = GetFuncIFaxEnable();						/* �C���^�[�l�b�g�t�@�N�X�@�\ */
	PcMailEnable = GetFuncPcMailEnable();					/* ���[���@�\ */
	FolderEnable = GetFuncFolderEnable();					/* �t�H���_�@�\ */
	FaxDimm = GetMachineFaxDIMM();							/* FCU�g�������� */
	DblSideScn = GetFunc2sideScn();							/* ���ʓǂݎ��@�\ */
	Stamp = GetMachineStamp();								/* �σX�^���v�@�\ */
	SizeMix = GetFuncSizeMix();								/* �T�C�Y���ڋ@�\ */
	LsFunc = GetFuncLS();									/* LS�@�\ */
	PreviewEnable = datPV_PreviewEnable();					/* �v���r���[�@�\ */
	SMime = SMimeFuncGet();									/* S/MIME�@�\ */

/* @TohruShibuya_15S_0005 START */
    faxCpState = ModeCooperateStatGet();					/* FAX���[�h */
	cRetSPDoc = GetFuncSPDoc();								/* ���ꌴ�e�@�\ */
	iAutoDet = GetMachineAutoDetect();						/* �������m */
    a4model = GetMachineA4Model();							/* A4�@�픻�f */

	FAX_LOG("PrefDefPrgGet FunInfo[IFaxEnable=%d PcMailEnable=%d FolderEnable=%d FaxDimm=%d DblSideScn=%d Stamp=%d SizeMix=%d LsFunc=%d PreviewEnable=%d SMime=%d faxCpState =%d cRetSPDoc =%d iAutoDet =%d a4model=%d]\n",
			IFaxEnable, PcMailEnable, FolderEnable, FaxDimm, DblSideScn, Stamp, SizeMix, LsFunc, PreviewEnable, SMime,faxCpState,cRetSPDoc,iAutoDet,a4model);
	/* @Yamashita_11A12S_0005 END */

	/* ===== �v���t�@�����X�ݒ� ===== */
	/* <<<<< �ʏ��� >>>>> */
	/* ------------------------ */
	/* �W���u���[�h             */
	/* 0=���ڑ��M�A1=���������M */
	/* ------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[0], "%d", &DefNInfo.job_mode);
	WkStr[0] = PrefInfo->prefs_value[0];
	WkStr[1] = '\0';
	DefNInfo.job_mode = (u_char)strtoul(WkStr, &s, 10);

/* @TohruShibuya_15S_0005 START */
	/* [FAX�A�g�q�@]����[���ڑ��M]�̏ꍇ */
	if((faxCpState & FAXCP_STAT_CLIENT) && (DefNInfo.job_mode == FAXNV_DEF_JOBMODE_IMM) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_JOB_MODE, DefNInfo.job_mode);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet JOB_MODE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------------- */
	/* ��^���I��ԍ�                                                         */
	/* 0=��^�������A1=�y�}����z�A2=�y���}�z�A3=�y�֌W��ɔz�z���Ă��������z */
	/* 4=�y�d�b���������z�A5=�o�^��1�A6=�o�^��2�A7=�o�^��3                    */
	/* ---------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[1], "%d", &DefNInfo.string_num);

	WkStr[0] = PrefInfo->prefs_value[1];
	WkStr[1] = '\0';
	DefNInfo.string_num = (u_char)strtoul(WkStr, &s, 10);

	/* [FAX�A�g�q�@]����[��^���w�莞]�̏ꍇ */
	if((faxCpState & FAXCP_STAT_CLIENT) && (DefNInfo.string_num != FAXNV_DEF_FORM_NONE) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_STRING_NUM, DefNInfo.string_num);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet STRING_NUM = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------------- */
	/* ���[���{��                                                             */
	/* 0=�{�������A1=�{��No.1�A2=�{��No.2�A3=�{��No.3�A4=�{��No.4�A5=�{��No.5 */
	/* ---------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[2], "%d", &DefNInfo.mail_id);
	WkStr[0] = PrefInfo->prefs_value[2];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [�C���^�[�l�b�g�t�@�N�X�@�\����у��[���@�\�Ȃ�] �܂��� [FAX�A�g�q�@]����[�{���w�莞] */
	if((((IFaxEnable == DAT_OFF) && (PcMailEnable == DAT_OFF)) ||
		(faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS != TXOPT_OFF)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.mail_id = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_MAIL_ID, DefNInfo.mail_id);
	}
	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet MAIL_ID = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------- */
	/* �����T�C�Y(0=�ӂ����A1=�����Ȏ��A2=���׎�) */
	/* -------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[3], "%d", &DefNInfo.scan_set.str_size);
	WkStr[0] = PrefInfo->prefs_value[3];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [FCU�g���������Ȃ�]����[���׎��w�莞] */
	if( (FaxDimm == DAT_OFF) && (WkNumC == FAXNV_DEF_SCAN_RESOL_SFINE)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.str_size = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_STR_SIZE, DefNInfo.scan_set.str_size);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet STR_SIZE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------------------- */
	/* ���e���                                                                     */
	/* 0=�����A1=�����E�}�\�A2=�����E�ʐ^�A3=�ʐ^�A4=���[�U�[���e1�A5=���[�U�[���e2 */
	/* ---------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[4], "%d", &DefNInfo.scan_set.doc_kind);

	WkStr[0] = PrefInfo->prefs_value[4];
	WkStr[1] = '\0';
	DefNInfo.scan_set.doc_kind = (u_char)strtoul(WkStr, &s, 10);

	/* [���ꌴ�e�Ȃ�]����[���ꌴ�e�w�莞] */
	if((cRetSPDoc == DAT_OFF) && ((DefNInfo.scan_set.doc_kind  == FAXNV_DEF_SCAN_ORGNL_USER1) ||
	   (DefNInfo.scan_set.doc_kind == FAXNV_DEF_SCAN_ORGNL_USER2))  ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DOC_KIND, DefNInfo.scan_set.doc_kind);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet doc_kind ChkRet = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------- */
	/* �蓮�Z�x�l(0�`7) */
	/* ---------------- */
	//(void)sscanf(&PrefInfo->prefs_value[5], "%d", &DefNInfo.scan_set.notch);
	WkStr[0] = PrefInfo->prefs_value[5];
	WkStr[1] = '\0';
	DefNInfo.scan_set.notch = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_NOTCH, DefNInfo.scan_set.notch);

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet NOTCH = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------- */
	/* �����Z�x(0=OFF�A1=ON) */
	/* --------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[6], "%d", &DefNInfo.scan_set.auto_conc);
	WkStr[0] = PrefInfo->prefs_value[6];
	WkStr[1] = '\0';
	DefNInfo.scan_set.auto_conc = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_AUTO_CONC, DefNInfo.scan_set.auto_conc);

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet AUTO_CONC = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------- */
	/* �Ж�/����(0=�ЖʁA1=1���ڂ��痼�ʁA2=2���ڂ��痼��) */
	/* --------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[7], "%d", &DefNInfo.scan_set.dbl_scan.func);
	WkStr[0] = PrefInfo->prefs_value[7];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [���ʓǂݎ��@�\�Ȃ�] ���� [���ʎw�莞] */
	if((DblSideScn == DAT_OFF) && (WkNumC != FAXNV_DEF_SCAN_DBL_NONE)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.dbl_scan.func = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DBL_FUNC, DefNInfo.scan_set.dbl_scan.func);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SCAN_FUNC = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------- */
	/* �J������(0=���E�J���A1=�㉺�J��) */
	/* -------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[8], "%d", &DefNInfo.scan_set.dbl_scan.org_open);
	WkStr[0] = PrefInfo->prefs_value[8];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [���ʓǂݎ��@�\�Ȃ�] ���� [�㉺�J���w�莞] */
	if((DblSideScn == DAT_OFF) && (WkNumC== FAXNV_DEF_SCAN_ORGOPEN_UD)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.dbl_scan.org_open = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DBL_ORG_OPEN, DefNInfo.scan_set.dbl_scan.org_open);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet ORG_OPEN = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------- */
	/* �σX�^���v(0=OFF�A1=ON) */
	/* ----------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[9], "%d", &DefNInfo.scan_set.set_stamp);
	WkStr[0] = PrefInfo->prefs_value[9];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/* [�σX�^���v�@�\�Ȃ�]����[�σX�^���vON��] */
	if((Stamp == DAT_OFF) && (WkNumC == FAXNV_DEF_SCAN_STAMP_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.set_stamp = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SET_STAMP, DefNInfo.scan_set.set_stamp);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet STAMP = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------- */
	/* ���e�Z�b�g����(0=�ǂ߂�����A1=�ǂ߂Ȃ�����) */
	/* -------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[10], "%d", &DefNInfo.scan_set.org_set);
	WkStr[0] = PrefInfo->prefs_value[10];
	WkStr[1] = '\0';
	DefNInfo.scan_set.org_set = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_SET, DefNInfo.scan_set.org_set);


	/* ------------------------------------------------------------------------ */
	/* �ǂݎ��T�C�Y�ݒ���                                                   */
	/* 0=�������m�A1=��`�T�C�Y�A2=�o�^�T�C�Y1�A3=�o�^�T�C�Y2�A4=���e�T�C�Y���� */
	/* ------------------------------------------------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[11], "%d", &DefNInfo.scan_set.org_size.type);
	WkStr[0] = PrefInfo->prefs_value[11];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/* [�T�C�Y���ڋ@�\�Ȃ�] ���� [�T�C�Y���ڎw�莞] */
	if((SizeMix == DAT_OFF) && (WkNumC == FAXNV_SCAN_SIZE_MIX)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet type = FAXNV_SCAN_SIZE_MIX\n");
	}
	else{
		/* [�������m�@�\�Ȃ�] ���� [�������m�w�莞] */
		if((iAutoDet == DAT_OFF)&& (WkNumC == FAXNV_SCAN_SIZE_AUTO)){
			ChkRet |= PREF_RET_GET_ERR;
			FAX_LOG("PrefDefPrgSet type = FAXNV_SCAN_SIZE_AUTO\n");
		}else{
			DefNInfo.scan_set.org_size.type = WkNumC;
			ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_TYPE, DefNInfo.scan_set.org_size.type);
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SIZE_TIPE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------------------------------------------------- */
	/* �ǂݎ��T�C�Y��`�T�C�Y(�ő�2���̐���������)                                                 */
	/* 0=A4���R�A1=A4�^�e�A2=B4�^�e�A3=A3�^�e�A4=LT���R�A5=LT�^�e�A6=LG�^�e�A7=DLT�^�e               */
	/* 8=B4���R�A9=A3���R�A10=B3�^�e�A11=A2�^�e�A12=LG���R�A13=DLT���R�A14=17�~22�^�e�A15=A3�m�r�^�e */
	/* ��8-15�FSeaDragon�n                                                                           */
	/* --------------------------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[12], "%2d", &DefNInfo.scan_set.org_size.size);
	WkStr[0] = PrefInfo->prefs_value[12];
	WkStr[1] = PrefInfo->prefs_value[13];
	WkStr[2] = '\0';
	DefNInfo.scan_set.org_size.size = (u_char)strtoul(WkStr, &s, 10);
#if defined( SEADRAGON_C3 ) /* @Yamashita_11A_VGA_0015 */
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_SIZE_SD, DefNInfo.scan_set.org_size.size);
#else

	/* A4�@�̏ꍇ */
    if(a4model == DAT_ON){

    	/* A4�@��Ŏw��s�ȗp��(A4T�A8Hx14T�A8Hx14T�A8Hx11Y�ȊO) */
    	if	((DefNInfo.scan_set.org_size.size != FAXNV_SCAN_SIZE_A4T) ||
    	(DefNInfo.scan_set.org_size.size != FAXNV_SCAN_SIZE_8Hx14T) ||
    	(DefNInfo.scan_set.org_size.size != FAXNV_SCAN_SIZE_8Hx13_4T) ||
    	(DefNInfo.scan_set.org_size.size != FAXNV_SCAN_SIZE_8Hx11Y)){
    		ChkRet |= PREF_RET_GET_ERR;
    	}else{
        	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_SIZE, DefNInfo.scan_set.org_size.size);
    	}
    }else{
    	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_SIZE, DefNInfo.scan_set.org_size.size);
    }

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SIZE_SET = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

#endif

	/* ----------------------------------------------------------------- */
	/* ��T�C�Y���e�w��(0=OFF�A1=1���ڂ����T�C�Y�A2=2���ڂ����T�C�Y) */
	/* ----------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[14], "%d", &DefNInfo.scan_set.large_doc.func);
	WkStr[0] = PrefInfo->prefs_value[14];
	WkStr[1] = '\0';
	DefNInfo.scan_set.large_doc.func = (u_char)strtoul(WkStr, &s, 10);
#if defined( SEADRAGON_C3 ) /* @Yamashita_11A_VGA_0015 */
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_LARGE_DOC_FUNC_SD, DefNInfo.scan_set.large_doc.func);
#else
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_LARGE_DOC_FUNC, DefNInfo.scan_set.large_doc.func);
#endif

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LARGE_SIZE_SET = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------- */
	/* ��T�C�Y���e�w��T�C�Y(0=A2�A1=B3) */
	/* ---------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[15], "%d", &DefNInfo.scan_set.large_doc.org_size);
	WkStr[0] = PrefInfo->prefs_value[15];
	WkStr[1] = '\0';
	DefNInfo.scan_set.large_doc.org_size = (u_char)strtoul(WkStr, &s, 10);
#if defined( SEADRAGON_C3 ) /* @Yamashita_11A_VGA_0015 */
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_LARGE_DOC_SIZE_SD, DefNInfo.scan_set.large_doc.org_size);
#else
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_LARGE_DOC_SIZE, DefNInfo.scan_set.large_doc.org_size);
#endif

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LARGE_SIZEt = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ### ���M�I�v�V���� ### */
	/* --------------------------- */
	/* Bcc���M(0=OFF�A1=ON)        */
	/* FAXNV_DEF_TXOPT_BCCTX(1<<0) */
	/* --------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[16], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[16];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);


	/* [�C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� ���� ���[���@�\�Ȃ�] ���� [FAX�A�g�q�@] ���� [BCC���M�w�莞] */
	if((((IFaxEnable == DAT_OFF) && (PcMailEnable == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
    	ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* BCC���M ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_BCCTX;
		}
		/* BCC���M OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_BCCTX;
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet BCC ChkRet = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------------- */
	/* ���M���ʃ��|�[�g(0=OFF�A1=ON) */
	/* FAXNV_DEF_TXOPT_REPORT(1<<1)  */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[17], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[17];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

	/* [FAX�A�g�q�@] ���� [���M���ʃ��|�[�gON��] */
	if((faxCpState & FAXCP_STAT_CLIENT) &&(TxOptData == TXOPT_ON)){
    	ChkRet |= PREF_RET_GET_ERR;
	}else{
		/* ���M���ʃ��|�[�g ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_REPORT;
		}
		/* ���M���ʃ��|�[�g OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_REPORT;
		}
	}
	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet REPORT = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------------- */
	/* ��M�m�F(0=OFF�A1=ON)           */
	/* FAXNV_DEF_TXOPT_CHKRECEPT(1<<2) */
	/* ------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[18], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[18];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [�C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� ���� ���[���@�\�Ȃ�] �܂��� [FAX�A�g�q�@] ���� [��M�m�FON��]  */
	if((((IFaxEnable == DAT_OFF) && (PcMailEnable == DAT_OFF)) ||
		(faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
    	ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* ��M�m�F ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_CHKRECEPT;
		}
		/* ��M�m�F OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_CHKRECEPT;
		}
	}
	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet CHK_RECEPT = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------- */
	/* �����k��(0=OFF�A1=ON)            */
	/* FAXNV_DEF_TXOPT_AUTOREDUCE(1<<3) */
	/* -------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[19], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[19];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX�A�g�q�@]����[�����k�� OFF��] */
    if( (faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_OFF) ){
		FAX_LOG("PrefDefPrgSet TxOptData FAXCP_STAT_CLIENT = TXOPT_OFF\n");
    	ChkRet |= PREF_RET_GET_ERR;
    }else{
    	/* [A4�@��]����[�����k�� ON��] */
    	if((a4model == DAT_ON) &&(TxOptData == TXOPT_ON)){
    		FAX_LOG("PrefDefPrgSet TxOptData a4model = TXOPT_ON\n");
    		ChkRet |= PREF_RET_GET_ERR;
    	}else{

			ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);
			if(TxOptData == TXOPT_ON){
				DefNInfo.tx_option |= FAXNV_DEF_TXOPT_AUTOREDUCE;
			}
			/* �����k�� OFF */ /* @Yamashita_11A12S_0007 */
			else{
				DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_AUTOREDUCE;
			}
    	}
    }

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet AUTO_REDUCE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}
	/* -------------------------- */
	/* ID���M(0=OFF�A1=ON)        */
	/* FAXNV_DEF_TXOPT_IDTX(1<<4) */
	/* -------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[20], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[20];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX�A�g�q�@]����[ID���MON��] */
    if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
    }else{

		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* ID���M ON*/
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_IDTX;
		}
		/* ID���M OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_IDTX;
		}
    }

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet TXOPT_ID = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}
	/* ----------------------------------------- */
	/* �����~��(0=OFF�A1=�~�ς̂݁A2=�~�ρ{���M) */
	/* FAXNV_DEF_TXOPT_STRONLY(1<<5)�~�ς̂�     */
	/* FAXNV_DEF_TXOPT_STRTX  (1<<6)�~��+���M    */
	/* ----------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[21], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[21];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);


	/* [LS�@�\�Ȃ�] �܂���[FAX�A�g�q�@]����[�~��OFF�ȊO�w�莞] */
	if(((LsFunc == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS != TXOPT_STROFF) ){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT_STR, TxOptData);

		/* �����~�ρF�~�ς̂� */
		if(TxOptData == TXOPT_STRONLY){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_STRONLY;
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRTX;
		}
		/* �����~�ρF�~�ρ{���M */
		else if(TxOptData == TXOPT_STRTX){
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRONLY;
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_STRTX;
		}
		/* �����~�� OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRONLY;
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRTX;
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet TXOPT_STR = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------------- */
	/* F�R�[�h���M(0=OFF�A1=ON)      */
	/* FAXNV_DEF_TXOPT_FCODETX(1<<7) */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[22], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[22];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX�A�g�q�@]����[F�R�[�h���M�w�莞] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* F�R�[�h���M ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_FCODETX;
		}
		/* F�R�[�h���M OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_FCODETX;
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FCODE_TX = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}
	/* ----------------------------- */
	/* F�R�[�h���o��(0=OFF�A1=ON)  */
	/* FAXNV_DEF_TXOPT_FCODERX(1<<8) */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[23], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[23];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX�A�g�q�@]����[F�R�[�h���o���w�莞] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{

		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* F�R�[�h���o�� ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_FCODERX;
		}
		/* F�R�[�h���o�� OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_FCODERX;
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FCODE_RX = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}
	/* ------------------------------ */
	/* ������������(0=OFF�A1=ON)      */
	/* FAXNV_DEF_TXOPT_LABELINS(1<<9) */
	/* ------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[24], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[24];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX�A�g�q�@]����[�����������ݎw�莞] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* ������������ ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_LABELINS;
		}
		/* ������������ OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_LABELINS;
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LABEL_LINS = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------- */
	/* ����(0=OFF�A1=ON)           */
	/* FAXNV_DEF_TXOPT_SIGN(1<<10) */
	/* --------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[25], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[25];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [S/MIME�@�\�Ȃ�] �܂��� [���[���@�\�Ȃ�] �܂��� [FAX�A�g�q�@]����[����ON�w�莞] */
	if(((SMime == DAT_OFF) || (PcMailEnable == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* ���� ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_SIGN;
		}
		/* ���� ON */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_SIGN;
		}
	}


	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SIGN = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}
	/* --------------------------------- */
	/* �Í���(0=OFF�A1=ON)               */
	/* FAXNV_DEF_TXOPT_ENCRYPTION(1<<11) */
	/* --------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[26], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[26];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [S/MIME�@�\�Ȃ�] �܂��� [���[���@�\�Ȃ�] �܂��� [FAX�A�g�q�@]����[�Í���ON�w�莞] */
	if(((SMime == DAT_OFF) || (PcMailEnable == DAT_OFF) ||
		(faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* �Í��� ON*/
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_ENCRYPTION;
		}
		/* �Í��� OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_ENCRYPTION;
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet ENCRYPTION = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------- */
	/* �����w�著�M(0=OFF�A1=ON) */
	/* ------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[27], "%d", &DefNInfo.time_later.func);
	WkStr[0] = PrefInfo->prefs_value[27];
	WkStr[1] = '\0';
	DefNInfo.time_later.func = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TIME_LATER_FUNC, DefNInfo.time_later.func);

	/*  [FAX�A�g�q�@]����[�����w�著�M�w�莞] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (DefNInfo.time_later.func == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		/* ----------------------------------------------------------- */
		/* �����w�著�M����(24�����ŕ\����4���̐���������A0000�`2359) */
		/* ----------------------------------------------------------- */
		//(void)sscanf(&PrefInfo->prefs_value[28], "%2d", &DefNInfo.time_later.hour);
		WkStr[0] = PrefInfo->prefs_value[28];
		WkStr[1] = PrefInfo->prefs_value[29];
		WkStr[2] = '\0';
		DefNInfo.time_later.hour = (u_char)strtoul(WkStr, &s, 10);
		ChkRet |= PrefValueRangeChk(PREF_ID_R_DP_TIME_LATER_HOUR, DefNInfo.time_later.hour);
		//(void)sscanf(&PrefInfo->prefs_value[30], "%2d", &DefNInfo.time_later.min);
		WkStr[0] = PrefInfo->prefs_value[30];
		WkStr[1] = PrefInfo->prefs_value[31];
		WkStr[2] = '\0';
		DefNInfo.time_later.min = (u_char)strtoul(WkStr, &s, 10);
		ChkRet |= PrefValueRangeChk(PREF_ID_R_DP_TIME_LATER_MIN, DefNInfo.time_later.min);

	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet TX_TIME = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------------------------------------------------------------------------ */
	/* ���M�����̈�(�ő�2���̐���������)                                                        */
	/* 0=OFF�A1=��1���M�����́A2=��2���M�����́A3=��3���M�����́A4=��4���M�����́A5=��5���M������ */
	/* 6=��6���M�����́A7=��7���M�����́A8=��8���M�����́A9=��9���M�����́A10=��10���M������      */
	/* ------------------------------------------------------------------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[32], "%2d", &DefNInfo.send_agen);
	WkStr[0] = PrefInfo->prefs_value[32];
	WkStr[1] = PrefInfo->prefs_value[33];
	WkStr[2] = '\0';
	DefNInfo.send_agen = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SEND_AGEN, DefNInfo.send_agen);

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SEND_AGEN = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------------------------------------- */
	/* FAX������(�ő�2���̐���������)                                           */
	/* 1=G3�A2=G3�����A3=G3-1�A4=G3-1�����A5=G3-2�A6=G3-2�����A7=G3-3�A8=G3-3���� */
	/* 9=G3��A10=G3������A11=I-G3�A12=I-G3�����A13=G4�A14=H323�A15=SIP          */
	/* -------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[34], "%2d", &DefNInfo.dial_kind);
	WkStr[0] = PrefInfo->prefs_value[34];
	WkStr[1] = PrefInfo->prefs_value[35];
	WkStr[2] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	UcsFaxLine = CnvLineDtoU(WkNumC);			/* ����\����`�̉����ʂ�UCS��`�ɕϊ� */
	FaxLine = GetMachineFaxLine(UcsFaxLine);	/* UCS��`����L�� */
	/* ����Ȃ� */
	if(FaxLine == DAT_OFF){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.dial_kind = (u_char)strtoul(WkStr, &s, 10);
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DIAL_KIND, DefNInfo.dial_kind);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LINE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------------------------------------------- */
	/* �C���^�[�l�b�g�t�@�N�XSMTP�I��(0=SMTP�T�[�o�[�o�R����A1=SMTP�T�[�o�[�o�R���Ȃ�) */
	/* -------------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[36], "%d", &DialKindSmtp);
	WkStr[0] = PrefInfo->prefs_value[36];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/*  [FAX�A�g�q�@]����[SMTP�T�[�o�[�o�R���Ȃ�] */
	if((IFaxEnable == DAT_OFF) && (WkNumC == 1)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DialKindSmtp = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SMTP, DialKindSmtp);

		/* SMTP�I���F�o�R���Ȃ� */
		if(DialKindSmtp != 0){
			DefNInfo.dial_kind |= (1 << IFAX_SMTP);
		}
		/* SMTP�I���F�o�R���� */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.dial_kind &= ~(1 << IFAX_SMTP);
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet I_SMTP = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------- */
	/* ���[��SMTP�I��(0=SMTP�T�[�o�[�o�R����A1=SMTP�T�[�o�[�o�R���Ȃ�) */
	/* ---------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[37], "%d", &DialKindSmtp);
	WkStr[0] = PrefInfo->prefs_value[37];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* ���[���@�\�Ȃ� ���� �����l */
	if((PcMailEnable == DAT_OFF) && (WkNumC == 1)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		DialKindSmtp = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SMTP, DialKindSmtp);


		/* SMTP�I���F�o�R���Ȃ� */
		if(DialKindSmtp != 0){
			DefNInfo.dial_kind |= (1 << PCMAIL_SMTP);
		}
		/* SMTP�I���F�o�R���� */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.dial_kind &= ~(1 << PCMAIL_SMTP);
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet M_SMTP = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}


	/* --------------------------------------------------------------- */
	/* ������(0=FAX�A1=�C���^�[�l�b�g�t�@�N�X�A2=���[���A3=�t�H���_) */
	/* --------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[38], "%d", &DefNInfo.dial_mode);
	WkStr[0] = PrefInfo->prefs_value[38];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* �C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� ���� �����ʂ��C���^�[�l�b�g�t�@�N�X */
	if((IFaxEnable == DAT_OFF) && (WkNumC == DIALMODE_IFAX)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet WkNumC = DIALMODE_IFAX\n");
	}
	/* ���[���@�\�Ȃ� ���� �����ʂ����[�� */
	else if((PcMailEnable == DAT_OFF) && (WkNumC == DIALMODE_PCMAIL)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet WkNumC = DIALMODE_PCMAIL\n");
	}
	/* �t�H���_�@�\�Ȃ� ���� �����ʂ��t�H���_ */
	else if((FolderEnable == DAT_OFF) && (WkNumC == DIALMODE_FOLDER)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet WkNumC = DIALMODE_FOLDER\n");
	}
	else{
		DefNInfo.dial_mode = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DIAL_MODE, DefNInfo.dial_mode);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FAX_MODE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------------- */
	/* ���M�O�v���r���[(0=OFF�A1=ON) */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[39], "%d", &DefNInfo.tx_preview);
	WkStr[0] = PrefInfo->prefs_value[39];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/*  [�v���r���[�@�\�Ȃ�]�܂���[FAX�A�g�q�@]����[���M�O�v���r���[�w�莞] */
	if(((PreviewEnable == ePV_PREVIEW_FUNC_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumC == DAT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.tx_preview = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TX_PREVIEW, DefNInfo.tx_preview);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet PREVIEW  ChkRet = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------------------ */
	/* �t�@�C���`��(0=TIFF�A1=PDF�A2=PDF/A) */
	/* ------------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[40], "%d", &TxFileOpt);
	WkStr[0] = PrefInfo->prefs_value[40];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [���[���@�\�Ȃ� ���� �t�H���_�@�\�Ȃ�] �܂��� [FAX�A�g�q�@]����[TIFF�ȊO�w�莞]*/
	if((((PcMailEnable == DAT_OFF) && (FolderEnable == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)) && (TxFileOpt != TXOPT_FILETIFF)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxFileOpt = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXFILEOPT, TxFileOpt);

		/* @Yamashita_11A12S_0008 START */
		/* �t�@�C���`���FPDF */ /* @Yamashita_11A12S_0007 */
		if(TxFileOpt == TXOPT_FILEPDF){
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILETIFF;
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILEPDF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDFA;
		}
		/* �t�@�C���`���FPDF/A */ /* @Yamashita_11A12S_0007 */
		else if(TxFileOpt == TXOPT_FILEPDFA){
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILETIFF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDF;
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILEPDFA;
		}
		/* �t�@�C���`���FTIFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILETIFF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDFA;
		}
		/* @Yamashita_11A12S_0008 END */
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FILE_OPT = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}


	/* ---------------------------- */
	/* �f�W�^������PDF(0=OFF�A1=ON) */
	/* ---------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[41], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[41];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);
	/* [���[���@�\�Ȃ� ���� �t�H���_�@�\�Ȃ�] �܂��� [FAX�A�g�q�@]����[TIFF�ȊO�w�莞]*/
	if((((PcMailEnable == DAT_OFF) && (FolderEnable == DAT_OFF)) || 
		(faxCpState & FAXCP_STAT_CLIENT)) && (TxOptData == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_FILESIGN, TxOptData);

		/* �f�W�^������ ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILESIGN; /* @Yamashita_12S_0026 */
		}
		/* �f�W�^������ OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILESIGN; /* @Yamashita_12S_0026 */
		}
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FILESIGN = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* <<<<< �ȒP��� >>>>> */
	/* -------------------------------------------- */
	/* �����T�C�Y(0=�ӂ����A1=�����Ȏ��A2=���׎�) */
	/* -------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[42], "%d", &DefSInfo.scan_set.str_size);
	WkStr[0] = PrefInfo->prefs_value[42];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/* [FCU�g���������Ȃ�]�܂���[FAX�A�g�q�@]����[���׎��w�莞] */
	if( ((FaxDimm == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT))  && (DefSInfo.scan_set.str_size == FAXNV_DEF_SCAN_RESOL_SFINE)){
		ChkRet |= PREF_RET_GET_ERR;

	}
	else{
		DefSInfo.scan_set.str_size = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_STR_SIZE, DefSInfo.scan_set.str_size);
	}


	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet S_STR_SIZE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------- */
	/* �Ж�/����(0=�ЖʁA1=1���ڂ��痼�ʁA2=2���ڂ��痼��) */
	/* --------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[43], "%d", &DefSInfo.scan_set.dbl_scan.func);
	WkStr[0] = PrefInfo->prefs_value[43];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* ���ʓǂݎ��@�\�Ȃ� ���� �㉺�J���w�莞 */
	if((DblSideScn == DAT_OFF) && (WkNumC == FAXNV_DEF_SCAN_ORGOPEN_UD)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefSInfo.scan_set.dbl_scan.func = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DBL_FUNC, DefSInfo.scan_set.dbl_scan.func);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet S_SCAAN_FUNCt = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------------------- */
	/* ������(0=FAX�A1=�C���^�[�l�b�g�t�@�N�X�A2=���[���A3=�t�H���_) */
	/* --------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[44], "%d", &DefSInfo.dial_mode);
	WkStr[0] = PrefInfo->prefs_value[44];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* �C���^�[�l�b�g�t�@�N�X�@�\�Ȃ� ���� �����ʂ��C���^�[�l�b�g�t�@�N�X */
	if((IFaxEnable == DAT_OFF) && (WkNumC == DIALMODE_IFAX)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet S_WkNumC = DIALMODE_IFAX\n");
	}
	/* ���[���@�\�Ȃ� ���� �����ʂ����[�� */
	else if((PcMailEnable == DAT_OFF) && (WkNumC == DIALMODE_PCMAIL)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet S_WkNumC = DIALMODE_PCMAIL\n");
	}
	/* �t�H���_�@�\�Ȃ� ���� �����ʂ��t�H���_ */
	else if((FolderEnable == DAT_OFF) && (WkNumC == DIALMODE_FOLDER)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet S_WkNumC = DIALMODE_FOLDER\n");
	}
	else{
		DefSInfo.dial_mode = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DIAL_MODE, DefSInfo.dial_mode);
	}

	/* �߂�l���G���[�ł������ꍇ */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet S_FAX_MODE = %d\n",ChkRet);
		/* �G���[�ŏ����I�� */
		return(PREF_RET_GET_ERR);
	}


/* @TohruShibuya_15S_0005 END */

	/* �`�F�b�NOK */
	if(ChkRet == PREF_RET_OK){

		/* �f�t�H���g�v���O�������ݒ�(�ʏ���) */
		DefNRet = FaxNVDefaultSet(FAXNV_HOME_WINMODE_NORMAL, &DefNInfo);
		FAX_LOG("PrefDefPrgSet:FaxNVDefaultSet[Normal Ret=%d]\n",DefNRet);
		/* �f�t�H���g�v���O�������ݒ�(�ȒP���) */
		DefSRet = FaxNVDefaultSet(FAXNV_HOME_WINMODE_SIMPLE, &DefSInfo);
		FAX_LOG("PrefDefPrgSet:FaxNVDefaultSet[Simple Ret=%d]\n",DefSRet);

		/* �ݒ茋�ʁF���� */
		if((DefNRet == FAXNV_OK) && (DefSRet == FAXNV_OK)) {
			/* �ݒ萬�� */
			FAX_LOG("PrefDefPrgSet N[job_mode=%d string_num=%d mail_id=%d str_size=%d doc_kind=%d notch=%d auto_conc=%d dbl_scan.func=%d]\n",
					DefNInfo.job_mode, DefNInfo.string_num, DefNInfo.mail_id, DefNInfo.scan_set.str_size, DefNInfo.scan_set.doc_kind,
					DefNInfo.scan_set.notch, DefNInfo.scan_set.auto_conc, DefNInfo.scan_set.dbl_scan.func);
			FAX_LOG("PrefDefPrgSet N[org_open=%d set_stamp=%d org_set=%d org_size.type=%d org_size.size=%d large_doc.func=%d large_doc.org_size=%d tx_option=0x%04x]\n",
					DefNInfo.scan_set.dbl_scan.org_open, DefNInfo.scan_set.set_stamp, DefNInfo.scan_set.org_set, DefNInfo.scan_set.org_size.type,
					DefNInfo.scan_set.org_size.size, DefNInfo.scan_set.large_doc.func, DefNInfo.scan_set.large_doc.org_size, DefNInfo.tx_option);
			FAX_LOG("PrefDefPrgSet N[time_later.func=%d time_later.hour=%d time_later.min=%d send_agen=%d dial_kind=0x%02x dial_mode=%d tx_preview=%d tx_fileopt=%d]\n",
					DefNInfo.time_later.func, DefNInfo.time_later.hour, DefNInfo.time_later.min, DefNInfo.send_agen, DefNInfo.dial_kind,
					DefNInfo.dial_mode, DefNInfo.tx_preview, DefNInfo.scan_set.tx_fileopt);
			FAX_LOG("PrefDefPrgSet S[str_size=%d dbl_scan.func=%d dial_mode=%d]\n",
					DefSInfo.scan_set.str_size, DefSInfo.scan_set.dbl_scan.func, DefSInfo.dial_mode);
			Ret = PREF_RET_OK;
		}
		else {
			/* �ݒ�G���[ */
			FAX_LOG("PrefDefPrgSet FaxNVDefaultSet Error[DefNRet=%d DefSRet=%d]\n", DefNRet, DefSRet);
			Ret = PREF_RET_SET_ERR;
		}
	}
	/* �`�F�b�NNG */
	else{
		FAX_LOG("PrefDefPrgSet CheckError[ChkRet=%d]\n",ChkRet);
		Ret = PREF_RET_CHK_ERR;
	}

	return(Ret);

}

#endif /* FAX_SPEC_PREF */
