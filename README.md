# Baseline solutions for the Online Round of IDAO-2019

[`Baseline - simple.ipynb`](https://github.com/yandexdataschool/IDAO-2018-muon-id/blob/master/Baseline%20-%20simple.ipynb) contains a naive xgboost solution

[`Baseline - advanced.ipynb`](https://github.com/yandexdataschool/IDAO-2018-muon-id/blob/master/Baseline%20-%20advanced.ipynb) contains a primer on jagged arrays processing and creates the model file for Track 2

`track_2_baseline` contains an example submission for Track 2. Run
`make all` to build and the `baseline` binary will be accepting data
as `.csv` (not gzipped) at the standard input and writing predictions
to the standard output. Run `make compute_features` to build an
utility binary that preprocesses data and outputs the computed
features.

`scoring.py` contains an implementation of the quality metric

`utils.py` contains data reading and jagged arrays processing utilities

`Evaluate advanced baseline.ipynb` compares Python and C++ predictions for Track 2