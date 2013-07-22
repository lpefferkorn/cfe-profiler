cfe-profiler
============

Cfe-profiler is a CFEngine bundles profiler, 
which helps you to find which bundles are the most time consuming.

Sample output of CFEngine-3.4.4 default policies profiling:

    Cfe-profiler-0.1: a CFEngine profiler - http://www.loicp.eu/cfe-profiler

    *** Sorted by wall-clock time ***

    Time(s) Namespace            Type               Bundle
       7.01   default           agent                 main
       7.01   default           agent               sleep1
       6.01   default           agent               sleep2
       4.00   default           agent               sleep4


CFEngine versions tested
------------------------
*  3.4.4
*  3.5.0

[Documentation available here](http://www.loicp.eu/cfe-profiler)

