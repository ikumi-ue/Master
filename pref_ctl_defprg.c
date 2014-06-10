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
*   2010.11.26 新規作成
*   2012.07.03 指定キャラコード仕様変更対応(@Yamashita_13S_0003)
*	2014.05.29 インポート・エキスポトート制約解除(@TohruShibuya_15S_0005)
*
*******************************************************/

/* ========================================================================= */
/* インクルードファイル宣言                                                  */
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
/* 構造体定義                                                                */
/* ========================================================================= */


/* ========================================================================= */
/* 内部制御データ定義                                                        */
/* ========================================================================= */
/* 設定名称 */
#ifdef FAXAPL_UWVGA
STRNO PrefStrTbl_defPrg[] = {U_COM_UP00066};		/* 初期画面として登録 */
#else
STRNO PrefStrTbl_defPrg[] = {ANCOM_BT00451};		/* 初期値として登録 */
#endif

/* 送信オプション：機能ON/OFF */
#define TXOPT_OFF					0				/* OFF */
#define TXOPT_ON					1				/* ON */

/* 送信オプション：文書蓄積(0=OFF、1=蓄積のみ、2=蓄積＋送信) */
#define TXOPT_STROFF				0				/* OFF */
#define TXOPT_STRONLY				1				/* 蓄積のみ */
#define TXOPT_STRTX					2				/* 蓄積+送信 */

/* 送信オプション：ファイル形式 */
#define TXOPT_FILETIFF				0				/* TIFF */
#define TXOPT_FILEPDF				1				/* PDF */
#define TXOPT_FILEPDFA				2				/* PDFA */

/* オプション無し時の無効項目に対する無効値 */ /* @Yamashita_11A12S_0005 */
#define INVALID_VALUE_DEFPRG		9				/* 無効値 */


/* ========================================================================= */
/* 内部関数プロトタイプ宣言                                                  */
/* ========================================================================= */


/* ===================================================================== */
/*                                                                       */
/* ===================================================================== */
/*                                                                       */
/* 関数名称：PrefDefPrgGet                                               */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* 概　　要：デフォルトプログラム取得処理                                */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* パラメタ：u_char       SecretInfo  機密情報有無                       */
/*           PrefParam_t  *PrefInfo   プリファレンス情報                 */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* 説　　明：デフォルトプログラムのプリファレンスデータ取得処理          */
/*                                                                       */
/* ===================================================================== */
u_int PrefDefPrgGet(u_char SecretInfo, PrefParam_t *PrefInfo)
{
    /* ---------------------------------------------- */
    /* 変数宣言                                       */
    /* ---------------------------------------------- */
	int						DefNRet,DefSRet;		/* 取得結果 */
	short					PrefNameRet;			/* 設定名称文字列取得結果 */
	fax_default_normal_t	DefNInfo,DefSInfo;		/* デフォルトプログラム */
	u_char					SysLang = LANG_JP;		/* システム言語 */
	/* @Yamashita_11A12S_0005 START */
	u_char 					IFaxFunc;				/* インターネットファクス機能 *//* @TohruShibuya_15S_0005 */
	u_char 					PcMailFunc;				/* メール機能 *//* @TohruShibuya_15S_0005 */
	u_char 					FolderFunc;				/* フォルダ機能 *//* @TohruShibuya_15S_0005 */
	u_char 					FaxDimm;				/* FCU拡張メモリ */
	u_char 					DblSideScn;				/* 両面読み取り機能 */
	u_char 					Stamp;					/* 済スタンプ機能 */
	u_char 					SizeMix;				/* サイズ混載機能 */
	u_char 					LsFunc;					/* LS機能 */
	PV_PREVIEW_FUNC			PreviewEnable;			/* プレビュー機能 */
	int 					SMime;					/* S/MIME機能 */
	u_char 					FaxLine;				/* 回線有無 */
	gwUsrEnumLine			UcsFaxLine;				/* UCS定義回線 */
	u_char					DefFaxLine;				/* Default回線 */
	/* @Yamashita_11A12S_0005 END */
	int						StatRet = SUB_STAT_OFF;	/* 状態チェック結果 */ /* @Yamashita_12S_0029 */

/* @TohruShibuya_15S_0005 START */
    u_int					faxCpState = 0;			/* FAX状態情報 */
	extern gwDrConfig_t	    dcs_config;				/* コンフィギュレーション情報 */
	u_char					cRetSPDoc;				/* 特殊原稿機能情報 */
	int						iAutoDet;				/* 自動検知機能有無	*/
/* @TohruShibuya_15S_0005 END */

    /* ---------------------------------------------- */
    /* 処理                                           */
    /* ---------------------------------------------- */

	FAX_LOG("PrefDefPrgGet Export START\n");

	if(PrefInfo == NULL){
		return(PREF_RET_SET_ERR);
	}

	/* デフォルトプログラム情報取得(通常画面) */
	DefNRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_NORMAL, &DefNInfo);
	FAX_LOG("PrefDefPrgGet:FaxNVDefaultGet[Normal Ret=%d]\n",DefNRet);
	/* デフォルトプログラム情報取得(簡単画面) */
	DefSRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_SIMPLE, &DefSInfo);
	FAX_LOG("PrefDefPrgGet:FaxNVDefaultGet[Simple Ret=%d]\n",DefSRet);

	/* 取得結果：正常以外 */
	if((DefNRet != FAXNV_OK) && (DefSRet != FAXNV_OK)) {
		/* 取得エラー */
		FAX_LOG("PrefDefPrgGet FaxNVDefaultGet Error[DefNRet=%d DefSRet=%d]\n", DefNRet, DefSRet);
		return(PREF_RET_SET_ERR);
	}

	/* @Yamashita_12S_0029 START */
	/* OCS READYチェック */
	StatRet = ModeFaxStatChk(MAIN_STAT_OCSREADY);
	FAX_LOG("PrefDefPrgGet ModeFaxStatChk[StatRet=%d]\n", StatRet);
	/* OCS READY ON */
	if(StatRet == SUB_STAT_ON){
		/* プリファレンス設定名称 */
		PrefNameRet = gwOpGetString(PrefInfo->LanguageKind, APLI_ID_FAX, PrefStrTbl_defPrg[0], PrefInfo->prefs_name);
		/* 設定名称の文字コード */
		PrefInfo->name_char_code = LangToCharCode(PrefInfo->LanguageKind);
		FAX_LOG("PrefDefPrgGet gwOpGetString[PrefNameRet=%d name_char_code=%d]\n", PrefNameRet, PrefInfo->name_char_code);
		/* 名称取得NGの場合は実行要求で通知された言語ではなく自分で自機の言語を取得して名称を取得する */
		if(PrefNameRet != NORMAL_END){
			PrefNameRet = FaxStrOcsIdStrGet(PrefStrTbl_defPrg[0], PrefInfo->prefs_name);
			/* 言語指定でエクスポート自体をエラーにしてはいけないと言う仕様から、ここで取得できなくても処理続行 */
			/* 設定名称の文字コード */
			SysLang = GetLangType();
			PrefInfo->name_char_code = LangToCharCode(SysLang);
			FAX_LOG("PrefDefPrgGet FaxStrOcsIdStrGet[PrefNameRet=%d name_char_code=%d]\n", PrefNameRet, PrefInfo->name_char_code);
		}
	}
	/* OCS READY OFF */
	else{
		/* PrefInfo->prefs_name は既にbzero済みのためこのままでよい */
		/* 設定名称の文字コード */
		PrefInfo->name_char_code = CHAR_CODE_NONE;
	}
	/* @Yamashita_12S_0029 END */

	/* 設定値の文字コード */
#if FAX_SPEC_13S /* @Yamashita_13S_0003 */
	PrefInfo->value_char_code = CHAR_CODE_EUSTD;
#else
	PrefInfo->value_char_code = CHAR_CODE_NONE;
#endif
	FAX_LOG("PrefDefPrgGet char_code[name=%d value=%d]\n", PrefInfo->name_char_code, PrefInfo->value_char_code);

	/* @Yamashita_11A12S_0005 START */
	/* ===== 機能情報取得 ===== */
	IFaxFunc = GetFuncIFaxFunc();							/* インターネットファクス機能 *//* @TohruShibuya_15S_0005 */
	PcMailFunc = GetFuncPcMailFunc();						/* メール機能 *//* @TohruShibuya_15S_0005 */
	FolderFunc = GetFuncFolderFunc();						/* フォルダ機能 *//* @TohruShibuya_15S_0005 */
	FaxDimm = GetMachineFaxDIMM();							/* FCU拡張メモリ */
	DblSideScn = GetFunc2sideScn();							/* 両面読み取り機能 */
	Stamp = GetMachineStamp();								/* 済スタンプ機能 */
	SizeMix = GetFuncSizeMix();								/* サイズ混載機能 */
	LsFunc = GetFuncLS();									/* LS機能 */
	PreviewEnable = datPV_PreviewEnable();					/* プレビュー機能 */
	SMime = SMimeFuncGet();									/* S/MIME機能 */
	UcsFaxLine = CnvLineDtoU(DefNInfo.dial_kind & 0x0f);	/* 宛先表示定義の回線種別をUCS定義に変換 */
	FaxLine = GetMachineFaxLine(UcsFaxLine);				/* UCS定義回線有無 */
	DefFaxLine = FaxLineDefault();							/* Default回線 */


/* @TohruShibuya_15S_0005 START */
    faxCpState = ModeCooperateStatGet();                     /* FAX状態 */
	cRetSPDoc = GetFuncSPDoc();                             /* 特殊原稿機能情報 */
	iAutoDet = GetMachineAutoDetect();                      /* 自動検知機能有無	*/
/* @TohruShibuya_15S_0005 END */

	FAX_LOG("PrefDefPrgGet FunInfo[IFaxFunc=%d PcMailFunc=%d FolderFunc=%d FaxDimm=%d DblSideScn=%d Stamp=%d SizeMix=%d LsFunc=%d PreviewEnable=%d SMime=%d FaxLine=%d faxCpState =%d cRetSPDoc =%d iAutoDet = %d]\n",
			IFaxFunc, PcMailFunc, FolderFunc, FaxDimm, DblSideScn, Stamp, SizeMix, LsFunc, PreviewEnable, SMime, FaxLine,faxCpState,cRetSPDoc,iAutoDet);
	/* @Yamashita_11A12S_0005 END */

	/* ===== プリファレンス設定 ===== */
	/* <<<<< 通常画面 >>>>> */


/* @TohruShibuya_15S_0005 START */
	/* ------------------------ */
	/* ジョブモード             */
	/* 0=直接送信、1=メモリ送信 */
	/* ------------------------ */
	/* ファクスオプション未搭載機の場合 */
	if(faxCpState & FAXCP_STAT_CLIENT){
		/* メモリ送信固定 */
		(void)sprintf(&PrefInfo->prefs_value[0], "%d", FAXNV_DEF_JOBMODE_MEM);
	}else{
		(void)sprintf(&PrefInfo->prefs_value[0], "%d", DefNInfo.job_mode);
	}

	/* ---------------------------------------------------------------------- */
	/* 定型文選択番号                                                         */
	/* 0=定型文無し、1=【マル秘】、2=【至急】、3=【関係区に配布してください】 */
	/* 4=【電話ください】、5=登録文1、6=登録文2、7=登録文3                    */
	/* ---------------------------------------------------------------------- */
	/* ファクスオプション未搭載機の場合 */
	if(faxCpState & FAXCP_STAT_CLIENT){
		/* 定型文なし */
		(void)sprintf(&PrefInfo->prefs_value[1], "%d",FAXNV_DEF_FORM_NONE);
	}else{
		(void)sprintf(&PrefInfo->prefs_value[1], "%d", DefNInfo.string_num);
	}

	/* ---------------------------------------------------------------------- */
	/* メール本文                                                             */
	/* 0=本文無し、1=本文No.1、2=本文No.2、3=本文No.3、4=本文No.4、5=本文No.5 */
	/* ---------------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [インターネットファクス機能なし 且つ メール機能なし]または[ファクスオプション未搭載機]または[HDDなし] */
	if(  ((IFaxFunc == DAT_OFF)&&(PcMailFunc == DAT_OFF)) ||
		 (faxCpState & FAXCP_STAT_CLIENT) || (dcs_config.max_typical_mail_body <= 0)){
		 /* 本文無し */
		(void)sprintf(&PrefInfo->prefs_value[2], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[2], "%d", DefNInfo.mail_id);
	}

	/* -------------------------------------------- */
	/* 文字サイズ(0=ふつう字、1=小さな字、2=微細字) */
	/* -------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [FCU拡張メモリなし]かつ[微細字選択]時 */
	if( (FaxDimm == DAT_OFF) && (DefNInfo.scan_set.str_size == FAXNV_DEF_SCAN_RESOL_SFINE) ){
		/* ふつう字 */
		(void)sprintf(&PrefInfo->prefs_value[3], "%d", FAXNV_DEF_SCAN_RESOL_STD);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[3], "%d", DefNInfo.scan_set.str_size);
	}

	/* ---------------------------------------------------------------------------- */
	/* 原稿種類                                                                     */
	/* 0=文字、1=文字・図表、2=文字・写真、3=写真、4=ユーザー原稿1、5=ユーザー原稿2 */
	/* ---------------------------------------------------------------------------- */

	/* [特殊原稿機能OFF]かつ[ユーザー原稿１、２]選択時 */
	if((cRetSPDoc == DAT_OFF) && ((DefNInfo.scan_set.doc_kind == FAXNV_DEF_SCAN_ORGNL_USER1) ||
		(DefNInfo.scan_set.doc_kind == FAXNV_DEF_SCAN_ORGNL_USER2))  ){
		/* 文字固定 */
		(void)sprintf(&PrefInfo->prefs_value[4], "%d", FAXNV_DEF_SCAN_ORGNL_TXT);
	}else{
		(void)sprintf(&PrefInfo->prefs_value[4], "%d", DefNInfo.scan_set.doc_kind);
	}

	/* ---------------- */
	/* 手動濃度値(0～7) */
	/* ---------------- */
	(void)sprintf(&PrefInfo->prefs_value[5], "%d", DefNInfo.scan_set.notch);

	/* --------------------- */
	/* 自動濃度(0=OFF、1=ON) */
	/* --------------------- */
	(void)sprintf(&PrefInfo->prefs_value[6], "%d", DefNInfo.scan_set.auto_conc);

	/* --------------------------------------------------- */
	/* 片面/両面(0=片面、1=1枚目から両面、2=2枚目から両面) */
	/* --------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [両面読み取り機能]なし */
	if(DblSideScn == DAT_OFF){
		/* 片面固定 */
		(void)sprintf(&PrefInfo->prefs_value[7], "%d", FAXNV_DEF_SCAN_DBL_NONE);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[7], "%d", DefNInfo.scan_set.dbl_scan.func);
	}

	/* -------------------------------- */
	/* 開き方向(0=左右開き、1=上下開き) */
	/* -------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [両面読み取り機能]なし */
	if(DblSideScn == DAT_OFF){
		/* 左右開き */
		(void)sprintf(&PrefInfo->prefs_value[8], "%d", FAXNV_DEF_SCAN_ORGOPEN_LR);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[8], "%d", DefNInfo.scan_set.dbl_scan.org_open);
	}

	/* ----------------------- */
	/* 済スタンプ(0=OFF、1=ON) */
	/* ----------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [済スタンプ機能]なし */
	if(Stamp == DAT_OFF){
		/* 済スタンプOFF */
		(void)sprintf(&PrefInfo->prefs_value[9], "%d", FAXNV_DEF_SCAN_STAMP_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[9], "%d", DefNInfo.scan_set.set_stamp);
	}

	/* -------------------------------------------- */
	/* 原稿セット方向(0=読める方向、1=読めない方向) */
	/* -------------------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[10], "%d", DefNInfo.scan_set.org_set);

	/* ------------------------------------------------------------------------ */
	/* 読み取りサイズ設定種別                                                   */
	/* 0=自動検知、1=定形サイズ、2=登録サイズ1、3=登録サイズ2、4=原稿サイズ混載 */
	/* ------------------------------------------------------------------------ */
	/* @Yamashita_11A12S_0005 */
	/* [自動検知機能]なし 且つ [自動検知] */
	if((iAutoDet == DAT_OFF)&& (DefNInfo.scan_set.org_size.type == FAXNV_SCAN_SIZE_AUTO)){
		/* 定形サイズ */
		(void)sprintf(&PrefInfo->prefs_value[11], "%d", FAXNV_SCAN_SIZE_STD);
	}else{
		/* [サイズ混載機能]なし 且つ [サイズ混載] */
		if((SizeMix == DAT_OFF) && (DefNInfo.scan_set.org_size.type == FAXNV_SCAN_SIZE_MIX)){
			/* 自動検知 */
			(void)sprintf(&PrefInfo->prefs_value[11], "%d", FAXNV_SCAN_SIZE_AUTO);
		}
		else{
			(void)sprintf(&PrefInfo->prefs_value[11], "%d", DefNInfo.scan_set.org_size.type);
		}
	}

	/* --------------------------------------------------------------------------------------------- */
	/* 読み取りサイズ定形サイズ(最大2桁の数字文字列)                                                 */
	/* 0=A4ヨコ、1=A4タテ、2=B4タテ、3=A3タテ、4=LTヨコ、5=LTタテ、6=LGタテ、7=DLTタテ               */
	/* 8=B4ヨコ、9=A3ヨコ、10=B3タテ、11=A2タテ、12=LGヨコ、13=DLTヨコ、14=17×22タテ、15=A3ノビタテ */
	/* --------------------------------------------------------------------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[12], "%2d", DefNInfo.scan_set.org_size.size);

	/* ----------------------------------------------------------------- */
	/* 大サイズ原稿指定(0=OFF、1=1枚目から大サイズ、2=2枚目から大サイズ) */
	/* ----------------------------------------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[14], "%d", DefNInfo.scan_set.large_doc.func);

	/* ---------------------------------- */
	/* 大サイズ原稿指定サイズ(0=A2、1=B3) */
	/* ---------------------------------- */
	(void)sprintf(&PrefInfo->prefs_value[15], "%d", DefNInfo.scan_set.large_doc.org_size);

	/* ### 送信オプション ### */
	/* --------------------------- */
	/* Bcc送信(0=OFF、1=ON)        */
	/* FAXNV_DEF_TXOPT_BCCTX(1<<0) */
	/* --------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [インターネットファクス機能なし 且つ メール機能なし]または [ファクスオプション未搭載機]*/
	if( ((IFaxFunc == DAT_OFF) && (PcMailFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF固定 */
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
	/* 送信結果レポート(0=OFF、1=ON) */
	/* FAXNV_DEF_TXOPT_REPORT(1<<1)  */
	/* ----------------------------- */
	/* ファクスオプション未搭載機の場合 */
	if(faxCpState & FAXCP_STAT_CLIENT){
		/* OFF固定 */
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
	/* 受信確認(0=OFF、1=ON)           */
	/* FAXNV_DEF_TXOPT_CHKRECEPT(1<<2) */
	/* ------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [インターネットファクス機能なし 且つ メール機能なし]または[ファクスオプション未搭載機] */
	if(((IFaxFunc == DAT_OFF) && (PcMailFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF固定 */
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
	/* 自動縮小(0=OFF、1=ON)            */
	/* FAXNV_DEF_TXOPT_AUTOREDUCE(1<<3) */
	/* -------------------------------- */
	/* ファクスオプション未搭載機の場合 */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* ON固定 */
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
	/* ID送信(0=OFF、1=ON)        */
	/* FAXNV_DEF_TXOPT_IDTX(1<<4) */
	/* -------------------------- */
	/* ファクスオプション未搭載機の場合 */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF固定 */
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
	/* 文書蓄積(0=OFF、1=蓄積のみ、2=蓄積＋送信) */
	/* FAXNV_DEF_TXOPT_STRONLY(1<<5)蓄積のみ     */
	/* FAXNV_DEF_TXOPT_STRTX  (1<<6)蓄積+送信    */
	/* ----------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [LS機能なし]または[ファクスオプション未搭載機] */
	if((LsFunc == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF固定 */
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
	/* Fコード送信(0=OFF、1=ON)      */
	/* FAXNV_DEF_TXOPT_FCODETX(1<<7) */
	/* ----------------------------- */
	/* ファクスオプション未搭載機の場合 */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF固定 */
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
	/* Fコード取り出し(0=OFF、1=ON)  */
	/* FAXNV_DEF_TXOPT_FCODERX(1<<8) */
	/* ----------------------------- */
	/* ファクスオプション未搭載機の場合 */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF固定 */
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
	/* 宛名差し込み(0=OFF、1=ON)      */
	/* FAXNV_DEF_TXOPT_LABELINS(1<<9) */
	/* ------------------------------ */
	/* ファクスオプション未搭載機の場合 */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* OFF固定 */
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
	/* 署名(0=OFF、1=ON)           */
	/* FAXNV_DEF_TXOPT_SIGN(1<<10) */
	/* --------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [S/MIME機能なし] または [メール機能なし]または[ファクスオプション未搭載機]  */
	if((SMime == DAT_OFF) || (PcMailFunc == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT) ){
		/* OFF固定 */
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
	/* 暗号化(0=OFF、1=ON)               */
	/* FAXNV_DEF_TXOPT_ENCRYPTION(1<<11) */
	/* --------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [S/MIME機能なし] または [メール機能なし]または[ファクスオプション未搭載機]  */
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
	/* 時刻指定送信              */
	/* ------------------------- */

	/* ファクスオプション未搭載機の場合 */
	if (faxCpState & FAXCP_STAT_CLIENT){
		/* ------------------------- */
		/* 時刻指定送信(0=OFF、1=ON) */
		/* ------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[27], "%d", TXOPT_OFF);

		/* ----------------------------------------------------------- */
		/* 時刻指定送信時分(24時制で表した4桁の数字文字列、0000～2359) */
		/* ----------------------------------------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[28], "%02d", 0);
		(void)sprintf(&PrefInfo->prefs_value[30], "%02d", 0);
	}else{
		/* ------------------------- */
		/* 時刻指定送信(0=OFF、1=ON) */
		/* ------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[27], "%d", DefNInfo.time_later.func);

		/* ----------------------------------------------------------- */
		/* 時刻指定送信時分(24時制で表した4桁の数字文字列、0000～2359) */
		/* ----------------------------------------------------------- */
		(void)sprintf(&PrefInfo->prefs_value[28], "%02d", DefNInfo.time_later.hour);
		(void)sprintf(&PrefInfo->prefs_value[30], "%02d", DefNInfo.time_later.min);
	}

	/* ------------------------------------------------------------------------------------------ */
	/* 発信元名称印字(最大2桁の数字文字列)                                                        */
	/* 0=OFF、1=第1発信元名称、2=第2発信元名称、3=第3発信元名称、4=第4発信元名称、5=第5発信元名称 */
	/* 6=第6発信元名称、7=第7発信元名称、8=第8発信元名称、9=第9発信元名称、10=第10発信元名称      */
	/* ------------------------------------------------------------------------------------------ */
	(void)sprintf(&PrefInfo->prefs_value[32], "%2d", DefNInfo.send_agen);

	/* -------------------------------------------------------------------------- */
	/* FAX回線種別(最大2桁の数字文字列):宛先表示定義                              */
	/* 1=G3、2=G3内線、3=G3-1、4=G3-1内線、5=G3-2、6=G3-2内線、7=G3-3、8=G3-3内線 */
	/* 9=G3空、10=G3内線空、11=I-G3、12=I-G3内線、13=G4、14=H323、15=SIP          */
	/* -------------------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* 回線なし */
	if(FaxLine == DAT_OFF){
		(void)sprintf(&PrefInfo->prefs_value[34], "%2d", DefFaxLine); /* Default回線 */
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[34], "%2d", (DefNInfo.dial_kind & 0x0f));
	}

	/* -------------------------------------------------------------------------------- */
	/* インターネットファクスSMTP選択(0=SMTPサーバー経由する、1=SMTPサーバー経由しない) */
	/* -------------------------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* インターネットファクス機能なし */
	if(IFaxFunc == DAT_OFF){
		/* OFF固定 */
		(void)sprintf(&PrefInfo->prefs_value[36], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[36], "%d", (((DefNInfo.dial_kind) &(1 << IFAX_SMTP)) >> IFAX_SMTP));
	}

	/* ---------------------------------------------------------------- */
	/* メールSMTP選択(0=SMTPサーバー経由する、1=SMTPサーバー経由しない) */
	/* ---------------------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* メール機能なし */
	if(PcMailFunc == DAT_OFF){
		/* OFF固定 */
		(void)sprintf(&PrefInfo->prefs_value[37], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[37], "%d", (((DefNInfo.dial_kind) & (1 << PCMAIL_SMTP)) >> PCMAIL_SMTP));
	}

	/* --------------------------------------------------------------- */
	/* 宛先種別(0=FAX、1=インターネットファクス、2=メール、3=フォルダ) */
	/* --------------------------------------------------------------- */
	 /* @Yamashita_11A12S_0005 */
	/* インターネットファクス 且つ インターネットファクス機能なし */
	if((DefNInfo.dial_mode == DIALMODE_IFAX) && (IFaxFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DIALMODE_FAX);
	}
	/* メール 且つ メール機能なし */
	else if((DefNInfo.dial_mode == DIALMODE_PCMAIL) && (PcMailFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DIALMODE_FAX);
	}
	/* フォルダ 且つ フォルダ機能なし */
	else if((DefNInfo.dial_mode == DIALMODE_FOLDER) && (FolderFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DIALMODE_FAX);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[38], "%d", DefNInfo.dial_mode);
	}

	/* ----------------------------- */
	/* 送信前プレビュー(0=OFF、1=ON) */
	/* ----------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [プレビュー機能なし]または[ファクスオプション未搭載機] */
	if((PreviewEnable == ePV_PREVIEW_FUNC_OFF) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF固定 */
		(void)sprintf(&PrefInfo->prefs_value[39], "%d", TXOPT_OFF);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[39], "%d", DefNInfo.tx_preview);
	}

	/* ------------------------------------ */
	/* ファイル形式(0=TIFF、1=PDF、2=PDF/A) */
	/* ------------------------------------ */
	/* @Yamashita_11A12S_0005 */
	/* [メール機能なし 且つ フォルダ機能なし]または[ファクスオプション未搭載機] */
	if( ((PcMailFunc == DAT_OFF) && (FolderFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* TIFF固定 */
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
	/* デジタル署名PDF(0=OFF、1=ON) */
	/* ---------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [メール機能なし 且つ フォルダ機能なし]または[ファクスオプション未搭載機] */
	if(((PcMailFunc == DAT_OFF) && (FolderFunc == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)){
		/* OFF固定 */
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

	/* <<<<< 簡単画面 >>>>> */
	/* -------------------------------------------- */
	/* 文字サイズ(0=ふつう字、1=小さな字、2=微細字) */
	/* -------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* [FCU拡張メモリなし]または[ファクスオプション未搭載機]かつ微細字選択時 */
	if(  ((FaxDimm == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (DefSInfo.scan_set.str_size == FAXNV_DEF_SCAN_RESOL_SFINE)){
		/* ふつう字 */
		(void)sprintf(&PrefInfo->prefs_value[42], "%d", FAXNV_DEF_SCAN_RESOL_STD);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[42], "%d", DefSInfo.scan_set.str_size);
	}

	/* --------------------------------------------------- */
	/* 片面/両面(0=片面、1=1枚目から両面、2=2枚目から両面) */
	/* --------------------------------------------------- */
	/* @Yamashita_11A12S_0005 */
	/* 両面読み取り機能なし */
	if(DblSideScn == DAT_OFF){
		 /* 片面 */
		(void)sprintf(&PrefInfo->prefs_value[43], "%d", FAXNV_DEF_SCAN_DBL_NONE);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[43], "%d", DefSInfo.scan_set.dbl_scan.func);
	}
/* @TohruShibuya_15S_0005 END */

	/* --------------------------------------------------------------- */
	/* 宛先種別(0=FAX、1=インターネットファクス、2=メール、3=フォルダ) */
	/* --------------------------------------------------------------- */
	 /* @Yamashita_11A12S_0005 */
	/* インターネットファクス 且つ インターネットファクス機能なし */
	if((DefNInfo.dial_mode == DIALMODE_IFAX) && (IFaxFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DIALMODE_FAX);
	}
	/* メール 且つ メール機能なし */
	else if((DefNInfo.dial_mode == DIALMODE_PCMAIL) && (PcMailFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DIALMODE_FAX);
	}
	/* フォルダ 且つ フォルダ機能なし */
	else if((DefNInfo.dial_mode == DIALMODE_FOLDER) && (FolderFunc == DAT_OFF)){
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DIALMODE_FAX);
	}
	else{
		(void)sprintf(&PrefInfo->prefs_value[44], "%d", DefSInfo.dial_mode);
	}

	/* 設定成功 */
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
/* 関数名称：PrefDefPrgSet                                               */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* 概　　要：デフォルトプログラム設定処理                                */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* パラメタ：u_char       SecretInfo  機密情報有無                       */
/*           PrefParam_t  *PrefInfo   プリファレンス情報                 */
/*                                                                       */
/* --------------------------------------------------------------------- */
/*                                                                       */
/* 説　　明：デフォルトプログラムのプリファレンスデータ設定処理          */
/*                                                                       */
/* ===================================================================== */
u_int PrefDefPrgSet(u_char SecretInfo, PrefParam_t *PrefInfo)
{
    /* ---------------------------------------------- */
    /* 変数宣言                                       */
    /* ---------------------------------------------- */
	int						DefNRet,DefSRet;		/* 取得結果 */
	u_int					Ret;					/* 実行結果 */
	u_int					ChkRet;					/* チェック結果 */
	fax_default_normal_t	DefNInfo,DefSInfo;		/* デフォルトプログラム */
	u_short					TxOptData = TXOPT_ON;	/* 送信オプションデータ */
	u_char					DialKindSmtp = 0;		/* SMTP選択情報 */
	u_char					TxFileOpt = 0;			/* ファイル形式 */
	u_char					WkStr[3];
	u_char					WkNumC;
	u_short					WkNumS;
	char					*s;
	/* @Yamashita_11A12S_0005 START */
	u_char 					IFaxEnable;				/* インターネットファクス機能 */
	u_char 					PcMailEnable;			/* メール機能 */
	u_char 					FolderEnable;			/* フォルダ機能 */
	u_char 					FaxDimm;				/* FCU拡張メモリ */
	u_char 					DblSideScn;				/* 両面読み取り機能 */
	u_char 					Stamp;					/* 済スタンプ機能 */
	u_char 					SizeMix;				/* サイズ混載機能 */
	u_char 					LsFunc;					/* LS機能 */
	PV_PREVIEW_FUNC			PreviewEnable;			/* プレビュー機能 */
	int 					SMime;					/* S/MIME機能 */
	u_char 					FaxLine;				/* 回線有無 */
	gwUsrEnumLine			UcsFaxLine;				/* UCS定義回線 */
	/* @Yamashita_11A12S_0005 END */


/* @TohruShibuya_15S_0005 START */
    u_int					faxCpState = 0;			/* FAX状態情報 */
	extern gwDrConfig_t		dcs_config;				/* コンフィギュレーション情報 */
	u_char					cRetSPDoc;				/* 特殊原稿機能情報 */
	int						iAutoDet;				/* 自動検知機能有無 */
    int						a4model;				/* A4機種判断 */

    /* ---------------------------------------------- */
    /* 処理                                           */
    /* ---------------------------------------------- */

	FAX_LOG("PrefDefPrgSet Import START\n");

	if(PrefInfo == NULL){
		return(PREF_RET_SET_ERR);
	}

	ChkRet = PREF_RET_OK;

	/* @Yamashita_11A12S_0005 START */
	/* -------------------------------------------------------------------- */
	/* オプション有無機間でのインポート対応のため、一端自機の情報を取得し   */
	/* その情報に対して条件判定しインポートデータを上書きしていく方法とする */
	/* -------------------------------------------------------------------- */
	/* デフォルトプログラム情報取得(通常画面) */
	DefNRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_NORMAL, &DefNInfo);
	FAX_LOG("PrefDefPrgSet:FaxNVDefaultGet[Normal Ret=%d]\n",DefNRet);
	/* デフォルトプログラム情報取得(簡単画面) */
	DefSRet = FaxNVDefaultGet(FAXNV_HOME_WINMODE_SIMPLE, &DefSInfo);
	FAX_LOG("PrefDefPrgSet:FaxNVDefaultGet[Simple Ret=%d]\n",DefSRet);
	/* 取得成功 */
	if((DefNRet != FAXNV_OK) && (DefSRet != FAXNV_OK)) {
		/* 取得エラー */
		FAX_LOG("PrefDefPrgSet FaxNVDefaultGet Error[DefNRet=%d DefSRet=%d]\n", DefNRet, DefSRet);
		return(PREF_RET_SET_ERR);
	}
	/* ===== 機能情報取得 ===== */
	IFaxEnable = GetFuncIFaxEnable();						/* インターネットファクス機能 */
	PcMailEnable = GetFuncPcMailEnable();					/* メール機能 */
	FolderEnable = GetFuncFolderEnable();					/* フォルダ機能 */
	FaxDimm = GetMachineFaxDIMM();							/* FCU拡張メモリ */
	DblSideScn = GetFunc2sideScn();							/* 両面読み取り機能 */
	Stamp = GetMachineStamp();								/* 済スタンプ機能 */
	SizeMix = GetFuncSizeMix();								/* サイズ混載機能 */
	LsFunc = GetFuncLS();									/* LS機能 */
	PreviewEnable = datPV_PreviewEnable();					/* プレビュー機能 */
	SMime = SMimeFuncGet();									/* S/MIME機能 */

/* @TohruShibuya_15S_0005 START */
    faxCpState = ModeCooperateStatGet();					/* FAXモード */
	cRetSPDoc = GetFuncSPDoc();								/* 特殊原稿機能 */
	iAutoDet = GetMachineAutoDetect();						/* 自動検知 */
    a4model = GetMachineA4Model();							/* A4機種判断 */

	FAX_LOG("PrefDefPrgGet FunInfo[IFaxEnable=%d PcMailEnable=%d FolderEnable=%d FaxDimm=%d DblSideScn=%d Stamp=%d SizeMix=%d LsFunc=%d PreviewEnable=%d SMime=%d faxCpState =%d cRetSPDoc =%d iAutoDet =%d a4model=%d]\n",
			IFaxEnable, PcMailEnable, FolderEnable, FaxDimm, DblSideScn, Stamp, SizeMix, LsFunc, PreviewEnable, SMime,faxCpState,cRetSPDoc,iAutoDet,a4model);
	/* @Yamashita_11A12S_0005 END */

	/* ===== プリファレンス設定 ===== */
	/* <<<<< 通常画面 >>>>> */
	/* ------------------------ */
	/* ジョブモード             */
	/* 0=直接送信、1=メモリ送信 */
	/* ------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[0], "%d", &DefNInfo.job_mode);
	WkStr[0] = PrefInfo->prefs_value[0];
	WkStr[1] = '\0';
	DefNInfo.job_mode = (u_char)strtoul(WkStr, &s, 10);

/* @TohruShibuya_15S_0005 START */
	/* [FAX連携子機]かつ[直接送信]の場合 */
	if((faxCpState & FAXCP_STAT_CLIENT) && (DefNInfo.job_mode == FAXNV_DEF_JOBMODE_IMM) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_JOB_MODE, DefNInfo.job_mode);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet JOB_MODE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------------- */
	/* 定型文選択番号                                                         */
	/* 0=定型文無し、1=【マル秘】、2=【至急】、3=【関係区に配布してください】 */
	/* 4=【電話ください】、5=登録文1、6=登録文2、7=登録文3                    */
	/* ---------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[1], "%d", &DefNInfo.string_num);

	WkStr[0] = PrefInfo->prefs_value[1];
	WkStr[1] = '\0';
	DefNInfo.string_num = (u_char)strtoul(WkStr, &s, 10);

	/* [FAX連携子機]かつ[定型文指定時]の場合 */
	if((faxCpState & FAXCP_STAT_CLIENT) && (DefNInfo.string_num != FAXNV_DEF_FORM_NONE) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_STRING_NUM, DefNInfo.string_num);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet STRING_NUM = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------------- */
	/* メール本文                                                             */
	/* 0=本文無し、1=本文No.1、2=本文No.2、3=本文No.3、4=本文No.4、5=本文No.5 */
	/* ---------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[2], "%d", &DefNInfo.mail_id);
	WkStr[0] = PrefInfo->prefs_value[2];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [インターネットファクス機能およびメール機能なし] または [FAX連携子機]かつ[本文指定時] */
	if((((IFaxEnable == DAT_OFF) && (PcMailEnable == DAT_OFF)) ||
		(faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS != TXOPT_OFF)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.mail_id = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_MAIL_ID, DefNInfo.mail_id);
	}
	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet MAIL_ID = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------- */
	/* 文字サイズ(0=ふつう字、1=小さな字、2=微細字) */
	/* -------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[3], "%d", &DefNInfo.scan_set.str_size);
	WkStr[0] = PrefInfo->prefs_value[3];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [FCU拡張メモリなし]かつ[微細字指定時] */
	if( (FaxDimm == DAT_OFF) && (WkNumC == FAXNV_DEF_SCAN_RESOL_SFINE)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.str_size = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_STR_SIZE, DefNInfo.scan_set.str_size);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet STR_SIZE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------------------- */
	/* 原稿種類                                                                     */
	/* 0=文字、1=文字・図表、2=文字・写真、3=写真、4=ユーザー原稿1、5=ユーザー原稿2 */
	/* ---------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[4], "%d", &DefNInfo.scan_set.doc_kind);

	WkStr[0] = PrefInfo->prefs_value[4];
	WkStr[1] = '\0';
	DefNInfo.scan_set.doc_kind = (u_char)strtoul(WkStr, &s, 10);

	/* [特殊原稿なし]かつ[特殊原稿指定時] */
	if((cRetSPDoc == DAT_OFF) && ((DefNInfo.scan_set.doc_kind  == FAXNV_DEF_SCAN_ORGNL_USER1) ||
	   (DefNInfo.scan_set.doc_kind == FAXNV_DEF_SCAN_ORGNL_USER2))  ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DOC_KIND, DefNInfo.scan_set.doc_kind);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet doc_kind ChkRet = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------- */
	/* 手動濃度値(0～7) */
	/* ---------------- */
	//(void)sscanf(&PrefInfo->prefs_value[5], "%d", &DefNInfo.scan_set.notch);
	WkStr[0] = PrefInfo->prefs_value[5];
	WkStr[1] = '\0';
	DefNInfo.scan_set.notch = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_NOTCH, DefNInfo.scan_set.notch);

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet NOTCH = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------- */
	/* 自動濃度(0=OFF、1=ON) */
	/* --------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[6], "%d", &DefNInfo.scan_set.auto_conc);
	WkStr[0] = PrefInfo->prefs_value[6];
	WkStr[1] = '\0';
	DefNInfo.scan_set.auto_conc = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_AUTO_CONC, DefNInfo.scan_set.auto_conc);

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet AUTO_CONC = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------- */
	/* 片面/両面(0=片面、1=1枚目から両面、2=2枚目から両面) */
	/* --------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[7], "%d", &DefNInfo.scan_set.dbl_scan.func);
	WkStr[0] = PrefInfo->prefs_value[7];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [両面読み取り機能なし] かつ [両面指定時] */
	if((DblSideScn == DAT_OFF) && (WkNumC != FAXNV_DEF_SCAN_DBL_NONE)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.dbl_scan.func = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DBL_FUNC, DefNInfo.scan_set.dbl_scan.func);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SCAN_FUNC = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------- */
	/* 開き方向(0=左右開き、1=上下開き) */
	/* -------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[8], "%d", &DefNInfo.scan_set.dbl_scan.org_open);
	WkStr[0] = PrefInfo->prefs_value[8];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [両面読み取り機能なし] かつ [上下開き指定時] */
	if((DblSideScn == DAT_OFF) && (WkNumC== FAXNV_DEF_SCAN_ORGOPEN_UD)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.dbl_scan.org_open = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DBL_ORG_OPEN, DefNInfo.scan_set.dbl_scan.org_open);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet ORG_OPEN = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------- */
	/* 済スタンプ(0=OFF、1=ON) */
	/* ----------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[9], "%d", &DefNInfo.scan_set.set_stamp);
	WkStr[0] = PrefInfo->prefs_value[9];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/* [済スタンプ機能なし]かつ[済スタンプON時] */
	if((Stamp == DAT_OFF) && (WkNumC == FAXNV_DEF_SCAN_STAMP_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.scan_set.set_stamp = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SET_STAMP, DefNInfo.scan_set.set_stamp);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet STAMP = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------- */
	/* 原稿セット方向(0=読める方向、1=読めない方向) */
	/* -------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[10], "%d", &DefNInfo.scan_set.org_set);
	WkStr[0] = PrefInfo->prefs_value[10];
	WkStr[1] = '\0';
	DefNInfo.scan_set.org_set = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_SET, DefNInfo.scan_set.org_set);


	/* ------------------------------------------------------------------------ */
	/* 読み取りサイズ設定種別                                                   */
	/* 0=自動検知、1=定形サイズ、2=登録サイズ1、3=登録サイズ2、4=原稿サイズ混載 */
	/* ------------------------------------------------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[11], "%d", &DefNInfo.scan_set.org_size.type);
	WkStr[0] = PrefInfo->prefs_value[11];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/* [サイズ混載機能なし] 且つ [サイズ混載指定時] */
	if((SizeMix == DAT_OFF) && (WkNumC == FAXNV_SCAN_SIZE_MIX)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet type = FAXNV_SCAN_SIZE_MIX\n");
	}
	else{
		/* [自動検知機能なし] 且つ [自動検知指定時] */
		if((iAutoDet == DAT_OFF)&& (WkNumC == FAXNV_SCAN_SIZE_AUTO)){
			ChkRet |= PREF_RET_GET_ERR;
			FAX_LOG("PrefDefPrgSet type = FAXNV_SCAN_SIZE_AUTO\n");
		}else{
			DefNInfo.scan_set.org_size.type = WkNumC;
			ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_TYPE, DefNInfo.scan_set.org_size.type);
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SIZE_TIPE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------------------------------------------------- */
	/* 読み取りサイズ定形サイズ(最大2桁の数字文字列)                                                 */
	/* 0=A4ヨコ、1=A4タテ、2=B4タテ、3=A3タテ、4=LTヨコ、5=LTタテ、6=LGタテ、7=DLTタテ               */
	/* 8=B4ヨコ、9=A3ヨコ、10=B3タテ、11=A2タテ、12=LGヨコ、13=DLTヨコ、14=17×22タテ、15=A3ノビタテ */
	/* ※8-15：SeaDragon系                                                                           */
	/* --------------------------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[12], "%2d", &DefNInfo.scan_set.org_size.size);
	WkStr[0] = PrefInfo->prefs_value[12];
	WkStr[1] = PrefInfo->prefs_value[13];
	WkStr[2] = '\0';
	DefNInfo.scan_set.org_size.size = (u_char)strtoul(WkStr, &s, 10);
#if defined( SEADRAGON_C3 ) /* @Yamashita_11A_VGA_0015 */
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_ORG_SIZE_SD, DefNInfo.scan_set.org_size.size);
#else

	/* A4機の場合 */
    if(a4model == DAT_ON){

    	/* A4機種で指定不可な用紙(A4T、8Hx14T、8Hx14T、8Hx11Y以外) */
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

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SIZE_SET = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

#endif

	/* ----------------------------------------------------------------- */
	/* 大サイズ原稿指定(0=OFF、1=1枚目から大サイズ、2=2枚目から大サイズ) */
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

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LARGE_SIZE_SET = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------- */
	/* 大サイズ原稿指定サイズ(0=A2、1=B3) */
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

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LARGE_SIZEt = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ### 送信オプション ### */
	/* --------------------------- */
	/* Bcc送信(0=OFF、1=ON)        */
	/* FAXNV_DEF_TXOPT_BCCTX(1<<0) */
	/* --------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[16], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[16];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);


	/* [インターネットファクス機能なし 且つ メール機能なし] 又は [FAX連携子機] かつ [BCC送信指定時] */
	if((((IFaxEnable == DAT_OFF) && (PcMailEnable == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
    	ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* BCC送信 ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_BCCTX;
		}
		/* BCC送信 OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_BCCTX;
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet BCC ChkRet = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------------- */
	/* 送信結果レポート(0=OFF、1=ON) */
	/* FAXNV_DEF_TXOPT_REPORT(1<<1)  */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[17], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[17];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

	/* [FAX連携子機] かつ [送信結果レポートON時] */
	if((faxCpState & FAXCP_STAT_CLIENT) &&(TxOptData == TXOPT_ON)){
    	ChkRet |= PREF_RET_GET_ERR;
	}else{
		/* 送信結果レポート ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_REPORT;
		}
		/* 送信結果レポート OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_REPORT;
		}
	}
	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet REPORT = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------------- */
	/* 受信確認(0=OFF、1=ON)           */
	/* FAXNV_DEF_TXOPT_CHKRECEPT(1<<2) */
	/* ------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[18], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[18];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [インターネットファクス機能なし かつ メール機能なし] または [FAX連携子機] かつ [受信確認ON時]  */
	if((((IFaxEnable == DAT_OFF) && (PcMailEnable == DAT_OFF)) ||
		(faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
    	ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* 受信確認 ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_CHKRECEPT;
		}
		/* 受信確認 OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_CHKRECEPT;
		}
	}
	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet CHK_RECEPT = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------- */
	/* 自動縮小(0=OFF、1=ON)            */
	/* FAXNV_DEF_TXOPT_AUTOREDUCE(1<<3) */
	/* -------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[19], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[19];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX連携子機]かつ[自動縮小 OFF時] */
    if( (faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_OFF) ){
		FAX_LOG("PrefDefPrgSet TxOptData FAXCP_STAT_CLIENT = TXOPT_OFF\n");
    	ChkRet |= PREF_RET_GET_ERR;
    }else{
    	/* [A4機種]かつ[自動縮小 ON時] */
    	if((a4model == DAT_ON) &&(TxOptData == TXOPT_ON)){
    		FAX_LOG("PrefDefPrgSet TxOptData a4model = TXOPT_ON\n");
    		ChkRet |= PREF_RET_GET_ERR;
    	}else{

			ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);
			if(TxOptData == TXOPT_ON){
				DefNInfo.tx_option |= FAXNV_DEF_TXOPT_AUTOREDUCE;
			}
			/* 自動縮小 OFF */ /* @Yamashita_11A12S_0007 */
			else{
				DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_AUTOREDUCE;
			}
    	}
    }

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet AUTO_REDUCE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}
	/* -------------------------- */
	/* ID送信(0=OFF、1=ON)        */
	/* FAXNV_DEF_TXOPT_IDTX(1<<4) */
	/* -------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[20], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[20];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX連携子機]かつ[ID送信ON時] */
    if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
    }else{

		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* ID送信 ON*/
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_IDTX;
		}
		/* ID送信 OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_IDTX;
		}
    }

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet TXOPT_ID = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}
	/* ----------------------------------------- */
	/* 文書蓄積(0=OFF、1=蓄積のみ、2=蓄積＋送信) */
	/* FAXNV_DEF_TXOPT_STRONLY(1<<5)蓄積のみ     */
	/* FAXNV_DEF_TXOPT_STRTX  (1<<6)蓄積+送信    */
	/* ----------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[21], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[21];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);


	/* [LS機能なし] または[FAX連携子機]かつ[蓄積OFF以外指定時] */
	if(((LsFunc == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS != TXOPT_STROFF) ){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT_STR, TxOptData);

		/* 文書蓄積：蓄積のみ */
		if(TxOptData == TXOPT_STRONLY){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_STRONLY;
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRTX;
		}
		/* 文書蓄積：蓄積＋送信 */
		else if(TxOptData == TXOPT_STRTX){
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRONLY;
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_STRTX;
		}
		/* 文書蓄積 OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRONLY;
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_STRTX;
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet TXOPT_STR = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------------- */
	/* Fコード送信(0=OFF、1=ON)      */
	/* FAXNV_DEF_TXOPT_FCODETX(1<<7) */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[22], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[22];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX連携子機]かつ[Fコード送信指定時] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* Fコード送信 ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_FCODETX;
		}
		/* Fコード送信 OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_FCODETX;
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FCODE_TX = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}
	/* ----------------------------- */
	/* Fコード取り出し(0=OFF、1=ON)  */
	/* FAXNV_DEF_TXOPT_FCODERX(1<<8) */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[23], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[23];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX連携子機]かつ[Fコード取り出し指定時] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{

		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* Fコード取り出し ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_FCODERX;
		}
		/* Fコード取り出し OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_FCODERX;
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FCODE_RX = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}
	/* ------------------------------ */
	/* 宛名差し込み(0=OFF、1=ON)      */
	/* FAXNV_DEF_TXOPT_LABELINS(1<<9) */
	/* ------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[24], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[24];
	WkStr[1] = '\0';
	TxOptData = (u_short)strtoul(WkStr, &s, 10);

	/* [FAX連携子機]かつ[宛名差し込み指定時] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (TxOptData == TXOPT_ON) ){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* 宛名差し込み ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_LABELINS;
		}
		/* 宛名差し込み OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_LABELINS;
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LABEL_LINS = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------- */
	/* 署名(0=OFF、1=ON)           */
	/* FAXNV_DEF_TXOPT_SIGN(1<<10) */
	/* --------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[25], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[25];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [S/MIME機能なし] または [メール機能なし] または [FAX連携子機]かつ[署名ON指定時] */
	if(((SMime == DAT_OFF) || (PcMailEnable == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* 署名 ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_SIGN;
		}
		/* 署名 ON */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_SIGN;
		}
	}


	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SIGN = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}
	/* --------------------------------- */
	/* 暗号化(0=OFF、1=ON)               */
	/* FAXNV_DEF_TXOPT_ENCRYPTION(1<<11) */
	/* --------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[26], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[26];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);

	/* [S/MIME機能なし] または [メール機能なし] または [FAX連携子機]かつ[暗号化ON指定時] */
	if(((SMime == DAT_OFF) || (PcMailEnable == DAT_OFF) ||
		(faxCpState & FAXCP_STAT_CLIENT)) && (WkNumS == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXOPT, TxOptData);

		/* 暗号化 ON*/
		if(TxOptData == TXOPT_ON){
			DefNInfo.tx_option |= FAXNV_DEF_TXOPT_ENCRYPTION;
		}
		/* 暗号化 OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.tx_option &= ~FAXNV_DEF_TXOPT_ENCRYPTION;
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet ENCRYPTION = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------- */
	/* 時刻指定送信(0=OFF、1=ON) */
	/* ------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[27], "%d", &DefNInfo.time_later.func);
	WkStr[0] = PrefInfo->prefs_value[27];
	WkStr[1] = '\0';
	DefNInfo.time_later.func = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TIME_LATER_FUNC, DefNInfo.time_later.func);

	/*  [FAX連携子機]かつ[時刻指定送信指定時] */
	if((faxCpState & FAXCP_STAT_CLIENT) && (DefNInfo.time_later.func == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}else{
		/* ----------------------------------------------------------- */
		/* 時刻指定送信時分(24時制で表した4桁の数字文字列、0000～2359) */
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

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet TX_TIME = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------------------------------------------------------------------------ */
	/* 発信元名称印字(最大2桁の数字文字列)                                                        */
	/* 0=OFF、1=第1発信元名称、2=第2発信元名称、3=第3発信元名称、4=第4発信元名称、5=第5発信元名称 */
	/* 6=第6発信元名称、7=第7発信元名称、8=第8発信元名称、9=第9発信元名称、10=第10発信元名称      */
	/* ------------------------------------------------------------------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[32], "%2d", &DefNInfo.send_agen);
	WkStr[0] = PrefInfo->prefs_value[32];
	WkStr[1] = PrefInfo->prefs_value[33];
	WkStr[2] = '\0';
	DefNInfo.send_agen = (u_char)strtoul(WkStr, &s, 10);
	ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SEND_AGEN, DefNInfo.send_agen);

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet SEND_AGEN = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------------------------------------- */
	/* FAX回線種別(最大2桁の数字文字列)                                           */
	/* 1=G3、2=G3内線、3=G3-1、4=G3-1内線、5=G3-2、6=G3-2内線、7=G3-3、8=G3-3内線 */
	/* 9=G3空、10=G3内線空、11=I-G3、12=I-G3内線、13=G4、14=H323、15=SIP          */
	/* -------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[34], "%2d", &DefNInfo.dial_kind);
	WkStr[0] = PrefInfo->prefs_value[34];
	WkStr[1] = PrefInfo->prefs_value[35];
	WkStr[2] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	UcsFaxLine = CnvLineDtoU(WkNumC);			/* 宛先表示定義の回線種別をUCS定義に変換 */
	FaxLine = GetMachineFaxLine(UcsFaxLine);	/* UCS定義回線有無 */
	/* 回線なし */
	if(FaxLine == DAT_OFF){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.dial_kind = (u_char)strtoul(WkStr, &s, 10);
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DIAL_KIND, DefNInfo.dial_kind);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet LINE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* -------------------------------------------------------------------------------- */
	/* インターネットファクスSMTP選択(0=SMTPサーバー経由する、1=SMTPサーバー経由しない) */
	/* -------------------------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[36], "%d", &DialKindSmtp);
	WkStr[0] = PrefInfo->prefs_value[36];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/*  [FAX連携子機]かつ[SMTPサーバー経由しない] */
	if((IFaxEnable == DAT_OFF) && (WkNumC == 1)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DialKindSmtp = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SMTP, DialKindSmtp);

		/* SMTP選択：経由しない */
		if(DialKindSmtp != 0){
			DefNInfo.dial_kind |= (1 << IFAX_SMTP);
		}
		/* SMTP選択：経由する */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.dial_kind &= ~(1 << IFAX_SMTP);
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet I_SMTP = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ---------------------------------------------------------------- */
	/* メールSMTP選択(0=SMTPサーバー経由する、1=SMTPサーバー経由しない) */
	/* ---------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[37], "%d", &DialKindSmtp);
	WkStr[0] = PrefInfo->prefs_value[37];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* メール機能なし 又は 無効値 */
	if((PcMailEnable == DAT_OFF) && (WkNumC == 1)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		DialKindSmtp = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_SMTP, DialKindSmtp);


		/* SMTP選択：経由しない */
		if(DialKindSmtp != 0){
			DefNInfo.dial_kind |= (1 << PCMAIL_SMTP);
		}
		/* SMTP選択：経由する */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.dial_kind &= ~(1 << PCMAIL_SMTP);
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet M_SMTP = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}


	/* --------------------------------------------------------------- */
	/* 宛先種別(0=FAX、1=インターネットファクス、2=メール、3=フォルダ) */
	/* --------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[38], "%d", &DefNInfo.dial_mode);
	WkStr[0] = PrefInfo->prefs_value[38];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* インターネットファクス機能なし 且つ 宛先種別がインターネットファクス */
	if((IFaxEnable == DAT_OFF) && (WkNumC == DIALMODE_IFAX)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet WkNumC = DIALMODE_IFAX\n");
	}
	/* メール機能なし 且つ 宛先種別がメール */
	else if((PcMailEnable == DAT_OFF) && (WkNumC == DIALMODE_PCMAIL)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet WkNumC = DIALMODE_PCMAIL\n");
	}
	/* フォルダ機能なし 且つ 宛先種別がフォルダ */
	else if((FolderEnable == DAT_OFF) && (WkNumC == DIALMODE_FOLDER)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet WkNumC = DIALMODE_FOLDER\n");
	}
	else{
		DefNInfo.dial_mode = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DIAL_MODE, DefNInfo.dial_mode);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FAX_MODE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ----------------------------- */
	/* 送信前プレビュー(0=OFF、1=ON) */
	/* ----------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[39], "%d", &DefNInfo.tx_preview);
	WkStr[0] = PrefInfo->prefs_value[39];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/*  [プレビュー機能なし]または[FAX連携子機]かつ[送信前プレビュー指定時] */
	if(((PreviewEnable == ePV_PREVIEW_FUNC_OFF) || (faxCpState & FAXCP_STAT_CLIENT)) && (WkNumC == DAT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefNInfo.tx_preview = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TX_PREVIEW, DefNInfo.tx_preview);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet PREVIEW  ChkRet = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* ------------------------------------ */
	/* ファイル形式(0=TIFF、1=PDF、2=PDF/A) */
	/* ------------------------------------ */
	//(void)sscanf(&PrefInfo->prefs_value[40], "%d", &TxFileOpt);
	WkStr[0] = PrefInfo->prefs_value[40];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* [メール機能なし かつ フォルダ機能なし] または [FAX連携子機]かつ[TIFF以外指定時]*/
	if((((PcMailEnable == DAT_OFF) && (FolderEnable == DAT_OFF)) || (faxCpState & FAXCP_STAT_CLIENT)) && (TxFileOpt != TXOPT_FILETIFF)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxFileOpt = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_TXFILEOPT, TxFileOpt);

		/* @Yamashita_11A12S_0008 START */
		/* ファイル形式：PDF */ /* @Yamashita_11A12S_0007 */
		if(TxFileOpt == TXOPT_FILEPDF){
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILETIFF;
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILEPDF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDFA;
		}
		/* ファイル形式：PDF/A */ /* @Yamashita_11A12S_0007 */
		else if(TxFileOpt == TXOPT_FILEPDFA){
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILETIFF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDF;
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILEPDFA;
		}
		/* ファイル形式：TIFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILETIFF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDF;
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILEPDFA;
		}
		/* @Yamashita_11A12S_0008 END */
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FILE_OPT = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}


	/* ---------------------------- */
	/* デジタル署名PDF(0=OFF、1=ON) */
	/* ---------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[41], "%d", &TxOptData);
	WkStr[0] = PrefInfo->prefs_value[41];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumS = (u_short)strtoul(WkStr, &s, 10);
	/* [メール機能なし かつ フォルダ機能なし] または [FAX連携子機]かつ[TIFF以外指定時]*/
	if((((PcMailEnable == DAT_OFF) && (FolderEnable == DAT_OFF)) || 
		(faxCpState & FAXCP_STAT_CLIENT)) && (TxOptData == TXOPT_ON)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{

		TxOptData = WkNumS;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_FILESIGN, TxOptData);

		/* デジタル署名 ON */
		if(TxOptData == TXOPT_ON){
			DefNInfo.scan_set.tx_fileopt |= FAXNV_DEF_TXOPT_FILESIGN; /* @Yamashita_12S_0026 */
		}
		/* デジタル署名 OFF */ /* @Yamashita_11A12S_0007 */
		else{
			DefNInfo.scan_set.tx_fileopt &= ~FAXNV_DEF_TXOPT_FILESIGN; /* @Yamashita_12S_0026 */
		}
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet FILESIGN = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* <<<<< 簡単画面 >>>>> */
	/* -------------------------------------------- */
	/* 文字サイズ(0=ふつう字、1=小さな字、2=微細字) */
	/* -------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[42], "%d", &DefSInfo.scan_set.str_size);
	WkStr[0] = PrefInfo->prefs_value[42];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);

	/* [FCU拡張メモリなし]または[FAX連携子機]かつ[微細字指定時] */
	if( ((FaxDimm == DAT_OFF) || (faxCpState & FAXCP_STAT_CLIENT))  && (DefSInfo.scan_set.str_size == FAXNV_DEF_SCAN_RESOL_SFINE)){
		ChkRet |= PREF_RET_GET_ERR;

	}
	else{
		DefSInfo.scan_set.str_size = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_STR_SIZE, DefSInfo.scan_set.str_size);
	}


	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet S_STR_SIZE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------- */
	/* 片面/両面(0=片面、1=1枚目から両面、2=2枚目から両面) */
	/* --------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[43], "%d", &DefSInfo.scan_set.dbl_scan.func);
	WkStr[0] = PrefInfo->prefs_value[43];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* 両面読み取り機能なし かつ 上下開き指定時 */
	if((DblSideScn == DAT_OFF) && (WkNumC == FAXNV_DEF_SCAN_ORGOPEN_UD)){
		ChkRet |= PREF_RET_GET_ERR;
	}
	else{
		DefSInfo.scan_set.dbl_scan.func = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DBL_FUNC, DefSInfo.scan_set.dbl_scan.func);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet S_SCAAN_FUNCt = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}

	/* --------------------------------------------------------------- */
	/* 宛先種別(0=FAX、1=インターネットファクス、2=メール、3=フォルダ) */
	/* --------------------------------------------------------------- */
	//(void)sscanf(&PrefInfo->prefs_value[44], "%d", &DefSInfo.dial_mode);
	WkStr[0] = PrefInfo->prefs_value[44];
	WkStr[1] = '\0';
	/* @Yamashita_11A12S_0005 */
	WkNumC = (u_char)strtoul(WkStr, &s, 10);
	/* インターネットファクス機能なし 且つ 宛先種別がインターネットファクス */
	if((IFaxEnable == DAT_OFF) && (WkNumC == DIALMODE_IFAX)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet S_WkNumC = DIALMODE_IFAX\n");
	}
	/* メール機能なし 且つ 宛先種別がメール */
	else if((PcMailEnable == DAT_OFF) && (WkNumC == DIALMODE_PCMAIL)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet S_WkNumC = DIALMODE_PCMAIL\n");
	}
	/* フォルダ機能なし 且つ 宛先種別がフォルダ */
	else if((FolderEnable == DAT_OFF) && (WkNumC == DIALMODE_FOLDER)){
		ChkRet |= PREF_RET_GET_ERR;
		FAX_LOG("PrefDefPrgSet S_WkNumC = DIALMODE_FOLDER\n");
	}
	else{
		DefSInfo.dial_mode = WkNumC;
		ChkRet |= PrefValueSelectChk(PREF_ID_S_DP_DIAL_MODE, DefSInfo.dial_mode);
	}

	/* 戻り値がエラーであった場合 */
	if(ChkRet != PREF_RET_OK){
		FAX_LOG("PrefDefPrgSet S_FAX_MODE = %d\n",ChkRet);
		/* エラーで処理終了 */
		return(PREF_RET_GET_ERR);
	}


/* @TohruShibuya_15S_0005 END */

	/* チェックOK */
	if(ChkRet == PREF_RET_OK){

		/* デフォルトプログラム情報設定(通常画面) */
		DefNRet = FaxNVDefaultSet(FAXNV_HOME_WINMODE_NORMAL, &DefNInfo);
		FAX_LOG("PrefDefPrgSet:FaxNVDefaultSet[Normal Ret=%d]\n",DefNRet);
		/* デフォルトプログラム情報設定(簡単画面) */
		DefSRet = FaxNVDefaultSet(FAXNV_HOME_WINMODE_SIMPLE, &DefSInfo);
		FAX_LOG("PrefDefPrgSet:FaxNVDefaultSet[Simple Ret=%d]\n",DefSRet);

		/* 設定結果：正常 */
		if((DefNRet == FAXNV_OK) && (DefSRet == FAXNV_OK)) {
			/* 設定成功 */
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
			/* 設定エラー */
			FAX_LOG("PrefDefPrgSet FaxNVDefaultSet Error[DefNRet=%d DefSRet=%d]\n", DefNRet, DefSRet);
			Ret = PREF_RET_SET_ERR;
		}
	}
	/* チェックNG */
	else{
		FAX_LOG("PrefDefPrgSet CheckError[ChkRet=%d]\n",ChkRet);
		Ret = PREF_RET_CHK_ERR;
	}

	return(Ret);

}

#endif /* FAX_SPEC_PREF */
