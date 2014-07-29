# Presentation

Cfe-profiler is a CFEngine bundles profiler.
It measures their execution time, helping you to find which bundles are the top time consumers.

If a bundle calls others bundles (through `methods:`), its total time will be the sum of all the called bundles.

Sample policy:

````
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
````

Output of cfe-profiler:

````
$ LD_PRELOAD=./cfe_profiler36.so cf-agent -Kf ./sample.cf

Cfe-profiler 0.3: a CFEngine profiler - http://www.loicp.eu/cfe-profiler

*** Sorted by wall-clock time ***

Time(s) Namespace Type            Bundle
   3.01   default agent           main
   3.01   default agent           sleep1
   2.00   default agent           sleep2
````

Output of cfe-profiler with the default CFEngine-3.6.1 policies:

````
$ LD_PRELOAD=./cfe_profiler36.so cf-agent -K

Cfe-profiler 0.3 : a CFEngine profiler - http://www.loicp.eu/cfe-profiler

*** Sorted by wall-clock time ***

Time(s) Namespace Type            Bundle
   1.77 default   agent           inventory_autorun
   0.97 default   agent           cfe_autorun_inventory_packages
   0.14 default   agent           cfe_autorun_inventory_proc
   0.12 default   agent           inventory_lsb
   0.12 default   common          paths
   0.09 default   agent           classic_services
   0.09 default   common          def
   0.08 default   common          inventory_control
   0.07 default   agent           cfe_autorun_inventory_dmidecode
   0.07 default   agent           cfe_internal_management
   0.05 default   agent           host_info_report_software
   0.05 default   common          inventory_linux
   0.04 default   agent           cfe_internal_update_folders
   0.02 default   agent           cfe_internal_update_policy
   0.02 default   agent           cfe_autorun_inventory_mtab
   0.02 default   agent           service_catalogue
   0.02 default   agent           change_management
   0.02 default   common          inventory_debian
   (...)
   ````
# Requirement

* A supported CFEngine version (see below)
* An adequate environment to compile cfe-profiler (gcc, a similar glibc to the target server)
* A cf-agent binary dynamically linked against libpromises.so (`ldd /path/to/cf-agent|grep -q libpromises.so && echo "cf-agent OK"`)

# Usage

* Fetch and compile the latest version of cfe-profiler:
````
$ wget https://github.com/lpefferkorn/cfe-profiler/archive/v0.3.tar.gz
$ tar xf v0.3.tar.gz
$ cd cfe-profiler-0.3/
$ make
````
* Set LD_PRELOAD environment variable and run cf-agent as usual, the above statistics will be displayed at the end of cf-agent execution:

````
$ LD_PRELOAD=/path/to/cfe_profiler36.so cf-agent
````

Depending your CFEngine version, use the appropriate cfe_profilerXX.so file (example: cfe_profiler34.so for 3.4.x versions, cfe_profiler36.so for 3.6.x and so on)

# Versions

Latest version is [cfe-profiler 0.3](https://github.com/lpefferkorn/cfe-profiler/archive/v0.3.tar.gz)

# Errors

````
cf-agent: /lib/x86_64-linux-gnu/libc.so.6: version `GLIBC_2.17' not found (required by ./cfe_profiler36.so)
````

It means that cfe-profiler has been compiled on a machine where the GNU Glibc differs from the one where cfe-profiler is used.

# Supported CFEngine versions

*  3.4.x
*  3.5 up to 3.5.3
*  3.6.1
*  cfe-profiler is going to be [bundled with CFEngine-3.6.x](https://dev.cfengine.com/issues/3607)

