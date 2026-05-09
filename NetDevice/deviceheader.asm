	idnt	"deviceheader.c"
	near	code
	opt o+,ol+,op+,oc+,ot+,oj+,ob+,om+
	section	"CODE",code
	public	_LibNull
	cnop	0,4
_LibNull
	movem.l	l3,-(a7)
	moveq	#0,d0
l3	reg
l5	equ	0
	rts
; stacksize=0
	public	_DeviceName
	public	_DeviceVersionString
	public	_DeviceInitTab
	cnop	0,4
l6
	dc.w	19196
	dc.l	l6
	dc.l	26+l6
	dc.b	128
	dc.b	1
	dc.b	3
	dc.b	0
	dc.l	_DeviceName
	dc.l	6+_DeviceVersionString
	dc.l	_DeviceInitTab
