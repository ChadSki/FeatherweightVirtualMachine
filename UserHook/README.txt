To build the LSP for FVM, decend into this directory and do a "nmake". You 
should have..

	fvmLsp.dll : The driver which has the hooked networking call 
	implementations.

	fvmInLsp.exe : The executable which should be run to install the LSP for 
	FVM for intercepting the networking calls.

As of now, we intercept the following calls..

WSPSendTo, WSPConnect and WSPBind.

What is essentially done is when we are inside a VM and we try to attempt any 
operation with 127.0.0.1, we replace the IP value with the IP of the particular 
VM, if one was provided at the time of VM creation. No changes are done when 
operating in the host environment.

P.S. : Due to change in the way headers are included between Windows 2000 and 
Windows XP, if you notice "nmake" failing on either platforms, try changing the 
order in which "ws2spi.h" and "windows.h" are included in the file "fvmLsp.cpp".

To make things easy try doing it this way if you see build failures with just 
"nmake".

On Windows XP
	'nmake "WINXP="' and
	
On Windows 2000
	'nmake "WIN2K=" 
	
