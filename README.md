Fan speed control with pwm0
**************************************

This is a tiny service to drive the Fan speed according to CPU Temp.
Higher Cpu temperature makes the Fan speed faster.

Currently there are 6 Cpu temp. and it can be adjusted to your Fan model.
This has been tested with snowfan model in NanoPi R2S and mainline kernel.

The mainline kernel must expose the pwm0.

Install instructions
====================

Run in the shell:

	make
	sudo make install

After the installation you can check if the service is running, by hearing the Fan spinning lower or checking the service:

* Status of the service

	sudo systemctl status fan-monitor

	● fan-monitor.service - FAN speed control
	     Loaded: loaded (/lib/systemd/system/fan-monitor.service; enabled; vendor preset: enabled)
	     Active: active (exited) since Tue 2020-10-06 18:14:34 UTC; 26min ago
	    Process: 536 ExecStart=/usr/local/bin/fan-monitor64 (code=exited, status=0/SUCCESS)
	   Main PID: 536 (code=exited, status=0/SUCCESS)
	      Tasks: 1 (limit: 1145)
	     Memory: 184.0K
	     CGroup: /system.slice/fan-monitor.service
	             └─538 /usr/local/bin/fan-monitor64
	
	Oct 06 18:14:34 nanopi-r2s systemd[1]: Starting FAN speed control...
	Oct 06 18:14:34 nanopi-r2s systemd[1]: Finished FAN speed control.


* Stop the service

	sudo systemctl stop fan-monitor


* Start the service

	sudo systemctl start fan-monitor