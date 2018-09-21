	.include "MPlayDef.s"

	.equ	se_w195_grp, voicegroup111
	.equ	se_w195_pri, 4
	.equ	se_w195_rev, reverb_set+50
	.equ	se_w195_mvl, 127
	.equ	se_w195_key, 0
	.equ	se_w195_tbs, 1
	.equ	se_w195_exg, 0
	.equ	se_w195_cmp, 1

	.section .rodata
	.global	se_w195
	.align	2

@********************** Track  1 **********************@

se_w195_1:
	.byte	KEYSH , se_w195_key+0
	.byte	TEMPO , 100*se_w195_tbs/2
	.byte		VOICE , 73
	.byte		BENDR , 12
	.byte		LFOS  , 40
	.byte		PAN   , c_v+7
	.byte		VOL   , 25*se_w195_mvl/mxv
	.byte		BEND  , c_v+0
	.byte	W03
	.byte		VOL   , 29*se_w195_mvl/mxv
	.byte	W03
	.byte		        33*se_w195_mvl/mxv
	.byte		N48   , An3 , v112
	.byte	W03
	.byte		VOL   , 40*se_w195_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+4
	.byte	W02
se_w195_1_000:
	.byte		VOL   , 45*se_w195_mvl/mxv
	.byte		MOD   , 8
	.byte	W03
	.byte		VOL   , 51*se_w195_mvl/mxv
	.byte	W03
	.byte	PEND
se_w195_1_001:
	.byte		VOL   , 56*se_w195_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W03
	.byte		VOL   , 62*se_w195_mvl/mxv
	.byte	W03
	.byte	PEND
se_w195_1_002:
	.byte		VOL   , 72*se_w195_mvl/mxv
	.byte		PAN   , c_v-4
	.byte	W03
	.byte		VOL   , 81*se_w195_mvl/mxv
	.byte	W03
	.byte	PEND
se_w195_1_003:
	.byte		VOL   , 92*se_w195_mvl/mxv
	.byte		PAN   , c_v-8
	.byte	W03
	.byte		VOL   , 100*se_w195_mvl/mxv
	.byte	W03
	.byte	PEND
	.byte		PAN   , c_v-4
	.byte	W06
	.byte		VOL   , 82*se_w195_mvl/mxv
	.byte		PAN   , c_v+0
	.byte	W03
	.byte		VOL   , 52*se_w195_mvl/mxv
	.byte	W03
	.byte		        25*se_w195_mvl/mxv
	.byte		MOD   , 0
	.byte		PAN   , c_v+4
	.byte	W03
	.byte		VOL   , 29*se_w195_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+7
	.byte	W02
	.byte		VOL   , 33*se_w195_mvl/mxv
	.byte		N72   , Gs3 , v112
	.byte	W03
	.byte		VOL   , 40*se_w195_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+4
	.byte	W02
	.byte	PATT
	 .word	se_w195_1_000
	.byte	PATT
	 .word	se_w195_1_001
	.byte	PATT
	 .word	se_w195_1_002
	.byte	PATT
	 .word	se_w195_1_003
	.byte		PAN   , c_v-4
	.byte	W06
	.byte		        c_v+0
	.byte	W06
	.byte		        c_v+4
	.byte	W03
	.byte		VOL   , 87*se_w195_mvl/mxv
	.byte	W03
	.byte		        75*se_w195_mvl/mxv
	.byte		PAN   , c_v+7
	.byte	W03
	.byte		VOL   , 62*se_w195_mvl/mxv
	.byte	W03
	.byte		        48*se_w195_mvl/mxv
	.byte		PAN   , c_v+4
	.byte	W03
	.byte		VOL   , 33*se_w195_mvl/mxv
	.byte	W03
	.byte		        25*se_w195_mvl/mxv
	.byte	W01
	.byte		PAN   , c_v+0
	.byte	W02
	.byte		VOL   , 10*se_w195_mvl/mxv
	.byte	W03
	.byte	W01
	.byte		PAN   , c_v-4
	.byte	W05
	.byte	FINE

@********************** Track  2 **********************@

se_w195_2:
	.byte	KEYSH , se_w195_key+0
	.byte		VOICE , 73
	.byte		VOL   , 25*se_w195_mvl/mxv
	.byte		PAN   , c_v-17
	.byte		N48   , Cn4 , v100
	.byte	W03
	.byte		VOL   , 29*se_w195_mvl/mxv
	.byte	W03
	.byte		        33*se_w195_mvl/mxv
	.byte	W03
	.byte		        40*se_w195_mvl/mxv
	.byte	W03
se_w195_2_000:
	.byte		VOL   , 45*se_w195_mvl/mxv
	.byte		MOD   , 8
	.byte	W03
	.byte		VOL   , 51*se_w195_mvl/mxv
	.byte	W03
	.byte	PEND
	.byte		        56*se_w195_mvl/mxv
	.byte	W03
	.byte		        62*se_w195_mvl/mxv
	.byte	W03
	.byte		        72*se_w195_mvl/mxv
	.byte	W03
	.byte		        81*se_w195_mvl/mxv
	.byte	W03
	.byte		        92*se_w195_mvl/mxv
	.byte	W03
	.byte		        100*se_w195_mvl/mxv
	.byte	W03
	.byte	W06
	.byte		        82*se_w195_mvl/mxv
	.byte	W03
	.byte		        52*se_w195_mvl/mxv
	.byte	W03
	.byte		        25*se_w195_mvl/mxv
	.byte		MOD   , 0
	.byte		N72   , Bn3 , v100
	.byte	W03
	.byte		VOL   , 29*se_w195_mvl/mxv
	.byte	W03
	.byte		        33*se_w195_mvl/mxv
	.byte	W03
	.byte		        40*se_w195_mvl/mxv
	.byte	W03
	.byte	PATT
	 .word	se_w195_2_000
	.byte		VOL   , 56*se_w195_mvl/mxv
	.byte	W03
	.byte		        62*se_w195_mvl/mxv
	.byte	W03
	.byte		        72*se_w195_mvl/mxv
	.byte	W03
	.byte		        81*se_w195_mvl/mxv
	.byte	W03
	.byte		        92*se_w195_mvl/mxv
	.byte	W03
	.byte		        100*se_w195_mvl/mxv
	.byte	W03
	.byte	W06
	.byte	W06
	.byte	W03
	.byte		        87*se_w195_mvl/mxv
	.byte	W03
	.byte		        75*se_w195_mvl/mxv
	.byte	W03
	.byte		        62*se_w195_mvl/mxv
	.byte	W03
	.byte		        48*se_w195_mvl/mxv
	.byte	W03
	.byte		        33*se_w195_mvl/mxv
	.byte	W03
	.byte		        25*se_w195_mvl/mxv
	.byte	W03
	.byte		        10*se_w195_mvl/mxv
	.byte	W03
	.byte	W06
	.byte	FINE

@******************************************************@
	.align	2

se_w195:
	.byte	2	@ NumTrks
	.byte	0	@ NumBlks
	.byte	se_w195_pri	@ Priority
	.byte	se_w195_rev	@ Reverb.

	.word	se_w195_grp

	.word	se_w195_1
	.word	se_w195_2

	.end
