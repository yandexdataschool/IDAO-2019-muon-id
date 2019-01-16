#pragma once
#include <iostream>

const std::string REFERENCE_HEADER = "id,ncl[0],ncl[1],ncl[2],ncl[3],avg_cs[0],avg_cs[1],avg_cs[2],avg_cs[3],ndof,MatchedHit_TYPE[0],MatchedHit_TYPE[1],MatchedHit_TYPE[2],MatchedHit_TYPE[3],MatchedHit_X[0],MatchedHit_X[1],MatchedHit_X[2],MatchedHit_X[3],MatchedHit_Y[0],MatchedHit_Y[1],MatchedHit_Y[2],MatchedHit_Y[3],MatchedHit_Z[0],MatchedHit_Z[1],MatchedHit_Z[2],MatchedHit_Z[3],MatchedHit_DX[0],MatchedHit_DX[1],MatchedHit_DX[2],MatchedHit_DX[3],MatchedHit_DY[0],MatchedHit_DY[1],MatchedHit_DY[2],MatchedHit_DY[3],MatchedHit_DZ[0],MatchedHit_DZ[1],MatchedHit_DZ[2],MatchedHit_DZ[3],MatchedHit_T[0],MatchedHit_T[1],MatchedHit_T[2],MatchedHit_T[3],MatchedHit_DT[0],MatchedHit_DT[1],MatchedHit_DT[2],MatchedHit_DT[3],Lextra_X[0],Lextra_X[1],Lextra_X[2],Lextra_X[3],Lextra_Y[0],Lextra_Y[1],Lextra_Y[2],Lextra_Y[3],NShared,Mextra_DX2[0],Mextra_DX2[1],Mextra_DX2[2],Mextra_DX2[3],Mextra_DY2[0],Mextra_DY2[1],Mextra_DY2[2],Mextra_DY2[3],FOI_hits_N,FOI_hits_X,FOI_hits_Y,FOI_hits_Z,FOI_hits_DX,FOI_hits_DY,FOI_hits_DZ,FOI_hits_T,FOI_hits_DT,FOI_hits_S,PT,P";

bool check_header(std::istream& stream) {
    std::string header;
    std::getline(stream, header);
    return (header == REFERENCE_HEADER);
}
