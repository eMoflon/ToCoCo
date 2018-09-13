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

# Comparison of Results

## Different Comparisons

There are two kinds of Comparisons:
* Comparison of an input to an output topology (``-io``)
* Comparison of two different output topologies (``-ab``)

### Example
To obtain a comparison of two different output topologies a and b after 5 minutes execution time execute the following:
```
./compareResults.sh file1 file2 test.dot -ab 5
```

To obtain a comparison of an input and an output topology after 10 and 14 minutes execute the following
```
 ./compareResultsInOut.sh <serial.csv> output.dot
```
If you prefer the results as a png append another parameter ``-toPNG``

# Calculate Combined Stretch and Plots
To calculate the combined stretch of multiple experiments just pass the path to the data as first parameter and the path to the evaluation folder as second parameter to the calculateCombinedStretch script.
```
./calculateCombinedStretch.sh /../evaluationsergebnisse /../topology-control-evaluation/evaluation
```
Naming in the data folder should be:

Folders : datacollection_\<TC\>

Serial Files inside Folders: serial_\<ID\>.csv