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