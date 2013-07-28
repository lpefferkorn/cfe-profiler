# Presentation

Cfe-profiler is a CFEngine bundles profiler.
It measures their execution time, helping you to find which bundles are the top time consumers.

If a bundle calls others bundles (through methods:), its total time will be the sum of all the called bundles.

Sample policy:

    body common control {
      bundlesequence => { "main" };
    }

    bundle agent main {
      methods:
        "b1"  usebundle =>  "sleep1";
    }


    bundle agent sleep1 {
      commands:
        "/usr/bin/sleep 1";

      methods:
        "b2"  usebundle =>  "sleep2";
    }

    bundle agent sleep2 {
      commands:
        "/usr/bin/sleep 2";
    }

Output of cfe-profiler:

    Cfe-profiler-0.2: a CFEngine profiler - http://www.loicp.eu/cfe-profiler

    *** Sorted by wall-clock time ***

    Time(s) Namespace            Type               Bundle
       3.01   default           agent                 main
       3.01   default           agent               sleep1
       2.00   default           agent               sleep2


# Requirement

* A supported CFEngine version (see below)
* Adequate environement to compile cfe-profiler
* A cf-agent binary dynamically linked against libpromises.so (`ldd /path/to/cf-agent|grep -q libpromises.so && echo "cf-agent OK"`)

# Usage

* Set LD_PRELOAD environment variable and run cf-agent as usual, statistics will be displayed at the end of cf-agent execution:

    LD_PRELOAD=/path/to/cfe_profiler35.so cf-agent

Depending your CFEngine version (3.4.x or 3.5.x), use the appropriate file (cfe_profiler34.so or cfe_profiler35.so)

# Supported CFEngine versions

*  3.4.x
*  3.5 up to 3.5.1

