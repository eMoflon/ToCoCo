# Evaluation System
The mote output of the sensors can be transformed with this command-line interface to usable information.

## Evaluations

A basic execution looks like the following:
```
./cli.php --testbed flocklab --evaluation prr --source mote-output.csv --destination prr.csv
```

### Supported testbeds
* [Flocklab](https://www.flocklab.ethz.ch/wiki/)
* [TIZ (TUDμNet)](http://www.tudunet.tu-darmstadt.de/)
* COOJA Simulator

### Supported evaluations
* Packet Reception Rate
* Enery Consumption (Ticks)
* Energy Consumption (mAh)
* Timeline
* Routing Graph

## Tips & Tricks
* You can directly open evaluation output after the calculation is finished by adding ``--open``
* The testbed's output files are not good for inspecting. Look at colored searchable output in your webbrowser: ``./cli.php --testbed flocklab --evaluation timeline --source mote-output.csv --keep-debugging-messages --open``
* merge several evaluations for aggregated results: ``./cli.php --evaluation prr --merge prr1.csv,prr2.csv --destination prr_aggregated.csv``

# Batch compilation (scripts/makeall.sh)

To build a number of sensor images having different topology control algorithms enabled, you can use the script *makeall.sh*.
All you need to configure is the algorithms to compile, which are determined by the variable `algorithmIdentifiers`.
The script finds the specific implementation file (`implementationFile`) and the integer ID of the algorithm (`appConfSelector`) based on the *app-conf-constants.h* file in *$ToCoCo/src*.
The implementation file may be missing (e.g., for the already built-in algorithms).
If you want to add a custom implementation file for an algorithm with identifier X, then create a constant COMPONENT_TOPOLOGYCONTROL_IMPL_FILE_X in *app-conf-constants.h*.
In contrast to the implementation file, the integer ID is mandatory.
For an algorithm with identifier X, the corresponding constant in *app-conf-constants.h* should be COMPONENT_TOPOLOGYCONTROL_X.

Example:
```
./makeall.sh
```

# Batch evaluation of testbed results (scripts/batchProcessTestbedResults.sh)

The script *batchProcessTestbedResults.sh* searches for .tar.gz files in the current directory, unpacks each of them, and invokes a number of scripts for each resulting *serial.csv* file.

Currently, the following scripts are executed per *serial.csv* file.
   * *makeSerialOutputProperCsv.sh*: 
      Creates the file *serial_excel.csv*, which can be opened directly in Excel. 
      In contast to *serial.csv*, the raw output is shown in a single cell.
      Futhermore, the formatting of links is improved by removing the suffix .0 of node IDs.
   * *compareResultsInOut.sh*
      Compare topologies before and after topology control. See below for details.
   * *collectTopologyControlRuntime.sh*
      Creates a CSV that summaries the runtime of topology control per node (*topologyControlRuntime.csv*).
   * *plotTopologyEvolution.sh*
      Plots the state of the topology every minute in a configurable time range.
      The time range is currently set to the duration of the FlockLab experiments (t=0,1,...,13[min]).

# Summarize sensor image sizes (scripts/collectSensorImageSizes.sh)

The script *collectSensorImageSizes.sh* creates a CSV file that aggregates the image sizes of the sensor images found in the current folder.
Image files are identified by their extension (.sky for TMote Sky nodes).

Example:
```
collectSensorImageSizes.sh
```

# Comparison of Results (scripts/compareResults.sh, scripts/compareResultsInOut.sh)

There are two kinds of Comparisons:
* Comparison of an input to an output topology (``-io``)
* Comparison of two different output topologies (``-ab``)

## Example
To obtain a comparison of two different output topologies a and b after 5 minutes execution time execute the following:
```
./compareResults.sh file1 file2 test.dot -ab 5
```

To obtain a comparison of an input and an output topology after 10 and 14 minutes execute the following
```
 ./compareResultsInOut.sh <serial.csv>
```
If you prefer the results as a png append the parameter ``-toPNG``

# Calculate combined stretch plots (scripts/calculateCombinedStretch.sh)
To calculate the combined stretch of multiple experiments just pass the path to the data as first parameter and the path to the evaluation folder as second parameter to the calculateCombinedStretch script.
```
./calculateCombinedStretch.sh /../evaluationsergebnisse /../topology-control-evaluation/evaluation
```
Naming in the data folder should be:

Folders : datacollection_\<TC\>

Serial Files inside Folders: serial_\<ID\>.csv