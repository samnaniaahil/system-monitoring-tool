# system-monitoring-tool
## Usage Instructions

<p><b>Compile with:</b> gcc -o mySystemStats systemMonitoringTool.c</p>
<p><b>Run with:</b> ./mySystemMonitoringTool</p>

<p>Program can also be run with one or more of the following flags:</p>
<ul>
  <li><em><b>--system</b></em> <br>display CPU and memory usage</li>
  <li><em><b>--user</b></em> <br>display users and their corresponding sessions</li>
  <li><em><b>--graphics</b></em> <br>display basic graphics for CPU usage</li>
  <li><em><b>--samples=N</b></em> <br>number of times stats are displayed; default is 10</li>
  <li><em><b>--tdelay=N</b></em> <br>number of seconds between each sample/refresh; default is 1</li>
</ul>
