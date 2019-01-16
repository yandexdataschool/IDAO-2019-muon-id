import sys
from itertools import repeat
import numpy as np
import pandas as pd
import xgboost

SIMPLE_FEATURE_COLUMNS = ['ncl[0]', 'ncl[1]', 'ncl[2]', 'ncl[3]', 'avg_cs[0]',
       'avg_cs[1]', 'avg_cs[2]', 'avg_cs[3]', 'ndof', 'MatchedHit_TYPE[0]',
       'MatchedHit_TYPE[1]', 'MatchedHit_TYPE[2]', 'MatchedHit_TYPE[3]',
       'MatchedHit_X[0]', 'MatchedHit_X[1]', 'MatchedHit_X[2]',
       'MatchedHit_X[3]', 'MatchedHit_Y[0]', 'MatchedHit_Y[1]',
       'MatchedHit_Y[2]', 'MatchedHit_Y[3]', 'MatchedHit_Z[0]',
       'MatchedHit_Z[1]', 'MatchedHit_Z[2]', 'MatchedHit_Z[3]',
       'MatchedHit_DX[0]', 'MatchedHit_DX[1]', 'MatchedHit_DX[2]',
       'MatchedHit_DX[3]', 'MatchedHit_DY[0]', 'MatchedHit_DY[1]',
       'MatchedHit_DY[2]', 'MatchedHit_DY[3]', 'MatchedHit_DZ[0]',
       'MatchedHit_DZ[1]', 'MatchedHit_DZ[2]', 'MatchedHit_DZ[3]',
       'MatchedHit_T[0]', 'MatchedHit_T[1]', 'MatchedHit_T[2]',
       'MatchedHit_T[3]', 'MatchedHit_DT[0]', 'MatchedHit_DT[1]',
       'MatchedHit_DT[2]', 'MatchedHit_DT[3]', 'Lextra_X[0]', 'Lextra_X[1]',
       'Lextra_X[2]', 'Lextra_X[3]', 'Lextra_Y[0]', 'Lextra_Y[1]',
       'Lextra_Y[2]', 'Lextra_Y[3]', 'NShared', 'Mextra_DX2[0]',
       'Mextra_DX2[1]', 'Mextra_DX2[2]', 'Mextra_DX2[3]', 'Mextra_DY2[0]',
       'Mextra_DY2[1]', 'Mextra_DY2[2]', 'Mextra_DY2[3]', 'FOI_hits_N', 'PT', 'P']
ID_COLUMN = "id"


def main():
    MODEL_NAME = "track_2_model.xgb"
    model = xgboost.Booster({'nthread': 1})
    model.load_model(MODEL_NAME)
    types = dict(zip(SIMPLE_FEATURE_COLUMNS, repeat(np.float32)))
    types[ID_COLUMN] = np.uint64
    data = pd.read_csv(sys.stdin, usecols=[ID_COLUMN] + SIMPLE_FEATURE_COLUMNS,
                       dtype=types, index_col=ID_COLUMN, chunksize=1000)
    sys.stdout.write("id,prediction\n")
    for chunk in data:
        predictions = model.predict(xgboost.DMatrix(chunk.values), output_margin=True).astype(
            np.float32)
        pd.DataFrame(data={"prediction": predictions}, index=chunk.index).to_csv(
            sys.stdout, index_label=ID_COLUMN, header=False)


if __name__ == "__main__":
    main()
