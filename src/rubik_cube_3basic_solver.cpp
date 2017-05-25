/*`
 *   Copyright 2017 Toby Liu
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "rubik_cube_solver.hpp"

#include <iostream>
#include <cassert>

using namespace rb;

struct PieceCoord {
    char row;
    char col;
};

static const PieceCoord edge_pieces[4] = {
    {1, 0}, {2, 1}, {1, 2}, {0, 1}
};

struct UpCrossCheckData {
    CUBE_FACE chk_face;
    int chk_edge_idx;
    int up_face_edge_idx;
    std::string moves;
};

static const UpCrossCheckData up_cross_check_data[5] = {
    {F, LE, LE, "L'"},
    {F, RE, RE, "R"},
    {D, UE, DE, "F2"},
    {F, UE, DE, "F"},
    {F, DE, DE, "F"},
};


std::string RubikCube3BasicSolver::DoSolve() {
    std::string moves;

    FindBestCubeOrientation();

    if (!IsUpCrossSolved())
        moves += SolveUpCross() + " ";

    if (!IsUpCornersSolved())
        moves += SolveUpCorners() + " ";

    if (!IsSecondLayerSolved())
        moves += SolveSecondLayer() + " ";

    if (!IsDownCrossSolved())
        moves += SolveDownCross() + " ";

    if (!IsDownCornersSolved())
        moves += SolveDownCorners();

    return moves;
}


// Step 1: Up Cross
bool RubikCube3BasicSolver::IsUpCrossSolved() {
    return (IsCrossOriented(U) && (GetCrossMatchCount(UE) == 4));
}


inline bool RubikCube3BasicSolver::IsCrossOriented(const CUBE_FACE& f) {
    char f_char = cube_.GetMappedFaceChar(f);
    return (cube_.GetPieceChar(f, 0, 1, false) == f_char &&
            cube_.GetPieceChar(f, 1, 0, false) == f_char &&
            cube_.GetPieceChar(f, 1, 2, false) == f_char &&
            cube_.GetPieceChar(f, 2, 1, false) == f_char);
}


inline bool RubikCube3BasicSolver::IsCornerOriented(const CUBE_FACE& f) {
    char f_char = cube_.GetMappedFaceChar(f);
    return (cube_.GetPieceChar(f, 0, 0, false) == f_char &&
            cube_.GetPieceChar(f, 0, 2, false) == f_char &&
            cube_.GetPieceChar(f, 2, 0, false) == f_char &&
            cube_.GetPieceChar(f, 2, 2, false) == f_char);
}


inline int RubikCube3BasicSolver::GetCrossMatchCount(const FACE_EDGE& fe) {
    int match_cnt = 0;
    const PieceCoord &ep = edge_pieces[fe];
    for (int i = (int)L; i <= (int)B; i ++)
        if (cube_.GetPieceChar((CUBE_FACE)i, ep.row, ep.col, false) ==
            cube_.GetMappedFaceChar((CUBE_FACE)i))
            match_cnt ++;
    return match_cnt;
}


std::string RubikCube3BasicSolver::SolveUpCross() {
    std::string moves;
    int prev_moves_len = -1;
    const char u_face = cube_.GetMappedFaceChar(U);

    while (!IsCrossOriented(U)) {
        prev_moves_len = -1;
        while (moves.length() != prev_moves_len) {
            prev_moves_len = moves.length();
            for (int i = 0; i < 5; i ++) {
                const UpCrossCheckData &d = up_cross_check_data[i];
                const PieceCoord &ep = edge_pieces[d.chk_edge_idx];
                if (cube_.GetPieceChar(d.chk_face, ep.row, ep.col, false) == u_face) {
                    const PieceCoord &u_ep = edge_pieces[d.up_face_edge_idx];
                    for (int i = 0; i < 3; i ++)
                    {
                        if (cube_.GetPieceChar(U, u_ep.row, u_ep.col, false) != u_face)
                            break;

                        moves += MoveCube("U");
                    }
                    moves += MoveCube(d.moves);
                }
            }
        }
        cube_.RotateCube(ROTATE);
    }

    int max_match_cnt = -1;
    int max_rotate_cnt = 0;
    for (int i = 0; i < 4; i ++) {
        int match_cnt = GetCrossMatchCount(UE);
        if (match_cnt > max_match_cnt) {
            max_match_cnt = match_cnt;
            max_rotate_cnt = i;
        }
        MoveCube("U");
    }
    for (int i = 0; i < max_rotate_cnt; i ++)
        moves += MoveCube("U");

    while (GetCrossMatchCount(UE) < 4) {
        while (cube_.GetPieceChar(F, 0, 1, false) == cube_.GetMappedFaceChar(F)) {
            cube_.RotateCube(ROTATE);
        }
        if (cube_.GetPieceChar(L, 0, 1, false) != cube_.GetMappedFaceChar(L)) {
            moves += MoveCube("F L U L' U2 F' U");
        } else if (cube_.GetPieceChar(R, 0, 1, false) != cube_.GetMappedFaceChar(R)) {
            moves += MoveCube("F' R' U' R U2 F U'");
        } else if (cube_.GetPieceChar(B, 0, 1, false) != cube_.GetMappedFaceChar(B)) {
            moves += MoveCube("F2 U2 F2 U2 F2");
        }
    }

    return cube_.CompressMoves(moves);
}


// Step 2: Up Corners
bool RubikCube3BasicSolver::IsUpCornersSolved() {
    const char u_face = cube_.GetMappedFaceChar(U);

    if (cube_.GetPieceChar(U, 0, 0, false) != u_face ||
        cube_.GetPieceChar(U, 0, 2, false) != u_face ||
        cube_.GetPieceChar(U, 2, 0, false) != u_face ||
        cube_.GetPieceChar(U, 2, 2, false) != u_face)
        return false;

    for (int i = (int)L; i <= (int)B; i ++)
        if (cube_.GetPieceChar((CUBE_FACE)i, 0, 0, false) != cube_.GetMappedFaceChar((CUBE_FACE)i))
            return false;

    return true;
}


std::string RubikCube3BasicSolver::SolveUpCorners() {
    std::string moves;
    const char u_face = cube_.GetMappedFaceChar(U);

    while (!IsUpCornersSolved()) {
        char f_face = cube_.GetMappedFaceChar(F);
        char r_face = cube_.GetMappedFaceChar(R);

        // Search the top-right corner of FRONT face on 3rd layer.
        // If any corner matches top-right corner of FRONT face,
        // move the corner to correct position.
        for (int i = 0; i < 4; i ++) {
            if (cube_.GetPieceChar(F, 2, 2, false) == u_face &&
                cube_.GetPieceChar(R, 2, 0, false) == r_face) {
                moves += MoveCube("F D F'");
                break;
            } else if (cube_.GetPieceChar(R, 2, 0, false) == u_face &&
                       cube_.GetPieceChar(F, 2, 2, false) == f_face) {
                moves += MoveCube("R' D' R");
                break;
            } else if (cube_.GetPieceChar(D, 0, 2, false) == u_face &&
                       cube_.GetPieceChar(F, 2, 2, false) == r_face &&
                       cube_.GetPieceChar(R, 2, 0, false) == f_face) {
                moves += MoveCube("F D' F' R' D2 R");
                break;
            }
            moves += MoveCube("D");
        }

        // The top-right corner of FRONT face is one of other UP face corners,
        // move the corner to the 3rd layer.
        if ((cube_.GetPieceChar(U, 2, 2, false) == u_face &&
             cube_.GetPieceChar(R, 0, 0, false) != r_face) ||
            cube_.GetPieceChar(F, 0, 2, false) == u_face ||
            cube_.GetPieceChar(R, 0, 0, false) == u_face) {
            moves += MoveCube("R' D' R");
        } else {
            cube_.RotateCube(ROTATE);
        }
    }

    return cube_.CompressMoves(moves);
}


// Step 3: Second Layer
bool RubikCube3BasicSolver::IsSecondLayerSolved() {
    for (int i = (int)L; i <= (int)B; i ++)
        if (cube_.GetPieceChar((CUBE_FACE)i, 1, 0, false) != cube_.GetMappedFaceChar((CUBE_FACE)i) ||
            cube_.GetPieceChar((CUBE_FACE)i, 1, 2, false) != cube_.GetMappedFaceChar((CUBE_FACE)i))
            return false;

    return true;
}


std::string RubikCube3BasicSolver::SolveSecondLayer() {
    static const int down_edges[4][2] = { {F, UE}, {L, LE}, {B, DE}, {R, RE} };

    std::string moves;
    char d_face = cube_.GetMappedFaceChar(D);

    while (!IsSecondLayerSolved()) {
        // Search the edges of 2nd layer on 3rd layer.
        // If any edge matches the current FRONT face,
        // rotate the edge to FRONT face and move it to right position.
        for (int i = 0; i < 4; i ++)
        {
            char f_face = cube_.GetMappedFaceChar(F);
            char l_face = cube_.GetMappedFaceChar(L);
            char r_face = cube_.GetMappedFaceChar(R);
            int prev_moves_len = -1;
            while (moves.length() != prev_moves_len) {
                prev_moves_len = moves.length();
                for (int i = 0; i < 4; i ++) {
                    CUBE_FACE chk_face  = (CUBE_FACE)down_edges[i][0];
                    const PieceCoord &d_ep = edge_pieces[down_edges[i][1]];
                    char edge_d_face = cube_.GetPieceChar(D, d_ep.row, d_ep.col, false);

                    if (cube_.GetPieceChar(chk_face, 2, 1, false) == f_face &&
                        edge_d_face != d_face) {
                        //std::cout << std::string("Move edge[") + f_face + ", " + edge_d_face + "] to 2nd layer" << std::endl;
                        // Rotate the matched edge to face FRONT
                        std::string rot_moves = std::string(i, 'D');
                        moves += MoveCube(rot_moves);

                        // Move the edge to correct position on 2nd layer
                        if (edge_d_face == l_face) {
                            moves += MoveCube("D L D' L' D' F' D F");
                        } else {
                            moves += MoveCube("D' R' D R D F D' F'");
                        }
                        break;
                    }
                }
            }
            cube_.RotateCube(ROTATE);
        }

        // Move the incorrect edges of 2nd layer to the 3rd layer
        for (int i = 0; i < 4; i ++) {
            char f_face = cube_.GetMappedFaceChar(F);
            char r_face = cube_.GetMappedFaceChar(R);
            char f_edge = cube_.GetPieceChar(F, 1, 2, false);
            char r_edge = cube_.GetPieceChar(R, 1, 0, false);

            if ((f_edge != f_face || r_edge != r_face) &&
                (f_edge != d_face && r_edge != d_face)) {
                //std::cout << std::string("Move edge[") + f_edge + ", " + r_edge + "] to 3rd layer" << std::endl;
                for (int j = 0; j < 3; j ++) {
                    if (cube_.GetPieceChar(D, 1, 0, false) == d_face ||
                        cube_.GetPieceChar(L, 2, 1, false) == d_face)
                        break;
                    moves += MoveCube("D");
                }
                moves += MoveCube("R' D R D F D' F'");
                break;
            }

            cube_.RotateCube(ROTATE);
        }
    }
    return cube_.CompressMoves(moves);
}


// Step 4: Down Cross
bool RubikCube3BasicSolver::IsDownCrossSolved() {
    return IsCrossOriented(D) && (GetCrossMatchCount(DE) == 4);
}


std::string RubikCube3BasicSolver::SolveDownCross() {
    std::string moves;
    char d_face = cube_.GetMappedFaceChar(D);

    // Solve DOWN face cross orientation
    while (!IsCrossOriented(D)) {
        if (cube_.GetPieceChar(D, 0, 1, false) != d_face) {
            if (cube_.GetPieceChar(D, 1, 0, false) != d_face) {
                //std::cout << "Solve L shape" << std::endl;
                moves += MoveCube("F D L D' L' F'");
            } else if (cube_.GetPieceChar(D, 2, 1, false) != d_face) {
                //std::cout << "Solve I shape" << std::endl;
                moves += MoveCube("F L D L' D' F'");
            }
        }
        cube_.RotateCube(ROTATE);
    }

    // Rotate DOWN face to match as many side faces as possible
    int max_match_cnt = -1;
    int max_rotate_cnt = 0;
    for (int i = 0; i < 4; i ++) {
        int match_cnt = GetCrossMatchCount(DE);
        if (match_cnt > max_match_cnt) {
            max_match_cnt = match_cnt;
            max_rotate_cnt = i;
        }
        MoveCube("D");
    }
    for (int i = 0; i < max_rotate_cnt; i ++)
        moves += MoveCube("D");

    // Solve DOWN face cross permutation
    if (GetCrossMatchCount(DE) < 4) {
        for (int i = 0; i < 3; i ++) {
            if (cube_.GetPieceChar(F, 2, 1, false) != cube_.GetMappedFaceChar(F)) {
                if (cube_.GetPieceChar(R, 2, 1, false) != cube_.GetMappedFaceChar(R) ||
                    cube_.GetPieceChar(B, 2, 1, false) != cube_.GetMappedFaceChar(B))
                    break;
            }
            cube_.RotateCube(ROTATE);
        }

        if (cube_.GetPieceChar(R, 2, 1, false) != cube_.GetMappedFaceChar(R)) {
            //std::cout << "Solve L shape permutation" << std::endl;
            moves += MoveCube("L D L' D L D2 L' D");
        } else {
            //std::cout << "Solve I shape permutation" << std::endl;
            moves += MoveCube("L D L' D L D2 L' D' L D L' D L D2 L'");
        }
    }
    assert(GetCrossMatchCount(DE) == 4);

    return cube_.CompressMoves(moves);
}


// Step 5: Down Corners
inline bool RubikCube3BasicSolver::IsDownCornerMatched() {
    char f_face = cube_.GetMappedFaceChar(F);
    char l_face = cube_.GetMappedFaceChar(L);
    char d_face = cube_.GetMappedFaceChar(D);
    int face_tag = (1 << CvtFaceCharToFace(f_face)) |
                   (1 << CvtFaceCharToFace(l_face)) |
                   (1 << CvtFaceCharToFace(d_face));

    char f_corner = cube_.GetPieceChar(F, 2, 0, false);
    char l_corner = cube_.GetPieceChar(L, 2, 2, false);
    char d_corner = cube_.GetPieceChar(D, 0, 0, false);
    int corner_tag = (1 << CvtFaceCharToFace(f_corner)) |
                   (1 << CvtFaceCharToFace(l_corner)) |
                   (1 << CvtFaceCharToFace(d_corner));

    return (face_tag == corner_tag);
}


inline int RubikCube3BasicSolver::GetDownCornerMatchCount() {
    int match_count = 0;
    for (int i = 0; i < 4; i ++) {
        if (IsDownCornerMatched())
            match_count ++;

        cube_.RotateCube(ROTATE);
    }
    return match_count;
}


bool RubikCube3BasicSolver::IsDownCornersSolved() {
    return (GetDownCornerMatchCount() == 4) && IsCornerOriented(D);
}


std::string RubikCube3BasicSolver::SolveDownCorners() {
    std::string moves;
    const char d_face = cube_.GetMappedFaceChar(D);

    // Solve DOWN face corners permutation
    while (GetDownCornerMatchCount() < 4) {
        for (int i = 0; i < 3; i ++) {
            if (IsDownCornerMatched())
                break;
            cube_.RotateCube(ROTATE);
        }
        //std::cout << "Change corners positions on 3rd layer" << std::endl;
        moves += MoveCube("D L D' R' D L' D' R");
    }

    // Solve DOWN face corners orientation
    while (!IsCornerOriented(D)) {
        for (int i = 0; i < 3; i ++) {
            if (cube_.GetPieceChar(D, 0, 0, false) != d_face)
                break;
            moves += MoveCube("D");
        }

        moves += MoveCube("L' U' L U L' U' L U");
    }
    for (int i = 0; i < 3; i ++) {
        if (cube_.GetPieceChar(F, 2, 0, false) == cube_.GetMappedFaceChar(F))
            break;
        moves += MoveCube("D");
    }

    return cube_.CompressMoves(moves);
}


void RubikCube3BasicSolver::FindBestCubeOrientation() {
    int max_score = 0;
    int max_orient_idx = 0;

    // Find the best orientation to start solving the cube
    for (int i = 0; i < 6; i ++) {
        int score = 0;
        if (IsCrossOriented(U)) {
            score ++;
            if (GetCrossMatchCount(UE) == 4) {
                score ++;
                if (IsUpCornersSolved()) {
                    score ++;
                    if (IsSecondLayerSolved()) {
                        score ++;
                        if (IsCrossOriented(D)) {
                            score ++;
                            if (GetCrossMatchCount(DE) == 4) {
                                score ++;
                                if (GetDownCornerMatchCount() == 4) {
                                    score ++;
                                    if (IsCornerOriented(D)) {
                                        score ++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (score > max_score) {
            max_score = score;
            max_orient_idx = i;
        }

        cube_.RotateCube(ROLL);
        if (i == 3) {
            cube_.RotateCube(ROTATE);
            cube_.RotateCube(ROLL);
        } else if (i == 4) {
            cube_.RotateCube(ROLL);
        }
    }
    cube_.RotateCube(ROTATE);
    cube_.RotateCube(ROTATE);
    cube_.RotateCube(ROTATE);
    assert(cube_.GetMappedFaceChar(U) == 'U');

    for (int i = 0; i < max_orient_idx; i ++) {
        cube_.RotateCube(ROLL);
        if (i == 3) {
            cube_.RotateCube(ROTATE);
            cube_.RotateCube(ROLL);
        } else if (i == 4) {
            cube_.RotateCube(ROLL);
        }
    }
}