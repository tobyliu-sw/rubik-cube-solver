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
#pragma once

#include "rubik_cube.hpp"

#include <string>
#include <cstring>
#include <cctype>
#include <cassert>

namespace rb {

class RubikCubeSolver {
  public:
    RubikCubeSolver(const RubikCube& cube): cube_(cube) {}
    virtual ~RubikCubeSolver() {}

    std::string Solve() { return DoSolve(); }

    char GetUpFaceChar() { return cube_.GetMappedFaceChar(U); }

  private:
    RubikCubeSolver(const RubikCubeSolver& other) {}
    RubikCubeSolver& operator=(const RubikCubeSolver& other) {}

    virtual std::string DoSolve() = 0;

  protected:
    virtual std::string MoveCube(const std::string& moves) {
        std::string ret_moves;

        cube_.Move(moves);

        for (int i = 0; i < moves.length(); i ++) {
            CUBE_FACE face = CvtFaceCharToFace(std::toupper(moves[i]));
            if (face == UNKNOWN_FACE) {
                ret_moves += moves[i];
            } else {
                ret_moves += cube_.GetMappedFaceChar(face);
            }
        }
        if (ret_moves.length())
            ret_moves += ' ';
        return ret_moves;
    }

    RubikCube cube_;
};

class RubikCube3BasicSolver: public RubikCubeSolver {
  public:
    RubikCube3BasicSolver(const RubikCube& cube):
        RubikCubeSolver(cube) { assert(cube_.GetDim() == 3); }

    // Step 1: Up Cross
    bool IsUpCrossSolved();
    std::string SolveUpCross();

    // Step 2: Up Corners
    bool IsUpCornersSolved();
    std::string SolveUpCorners();

    // Step 3: Second Layer
    bool IsSecondLayerSolved();
    std::string SolveSecondLayer();

    // Step 4: Down Cross
    bool IsDownCrossSolved();
    std::string SolveDownCross();

    // Step 5: Down Corners
    bool IsDownCornersSolved();
    std::string SolveDownCorners();

  private:
    std::string DoSolve();

    inline int GetCrossMatchCount(const FACE_EDGE& fe);
    inline bool IsCrossOriented(const CUBE_FACE& f);
    inline bool IsCornerOriented(const CUBE_FACE& f);

    inline bool IsDownCornerMatched();
    inline int GetDownCornerMatchCount();

    void FindBestCubeOrientation();
};

}