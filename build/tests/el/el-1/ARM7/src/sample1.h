
/*
 *  �ƥ�������ͥ���٤����
 */

#define MAIN_PRIORITY	5		/* �ᥤ�󥿥�����ͥ���� */
					/* HIGH_PRIORITY ���⤯���뤳�� */
#define HIGH_PRIORITY	9		/* ����˼¹Ԥ���륿������ͥ���� */
#define MID_PRIORITY	10
#define LOW_PRIORITY	11

/*
 *  �������åȰ�¸�������CPU�㳰�ϥ�ɥ�ε�ư��ˡ�ʤɡ�
 */

#ifndef STACK_SIZE
#define	STACK_SIZE	(8192)		/* �������Υ����å������� */
#endif /* STACK_SIZE */

/*
 *  �ؿ��Υץ�ȥ��������
 */
#ifndef _MACRO_ONLY

//extern void	main_task(VP_INT exinf);

#endif /* _MACRO_ONLY */
