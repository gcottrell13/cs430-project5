all:
	cl /MD /I. *.lib project.c /link /out:ezview.exe
