GraphAnalyzer is a tool for comparing topologies.

1. Compile using Makefile
2. Run by passing it 6 parameters:
    - input format: "-dot", "-graphML", or "-eval"
    - input file name 1
    - input file name 2
    - comparison mode "-ab" compares two output topologies, "-io" compares input to output topology
    - output format, same as input without eval
    - output file name