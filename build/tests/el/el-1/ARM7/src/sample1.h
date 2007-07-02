
/*
 *  各タスクの優先度の定義
 */

#define MAIN_PRIORITY	5		/* メインタスクの優先度 */
					/* HIGH_PRIORITY より高くすること */
#define HIGH_PRIORITY	9		/* 並列に実行されるタスクの優先度 */
#define MID_PRIORITY	10
#define LOW_PRIORITY	11

/*
 *  ターゲット依存の定義（CPU例外ハンドラの起動方法など）
 */

#ifndef STACK_SIZE
#define	STACK_SIZE	(8192)		/* タスクのスタックサイズ */
#endif /* STACK_SIZE */

/*
 *  関数のプロトタイプ宣言
 */
#ifndef _MACRO_ONLY

//extern void	main_task(VP_INT exinf);

#endif /* _MACRO_ONLY */
