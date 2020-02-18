//
//  CUMathBase.cpp
//  CUGL
//
//  Created by Walker White on 6/30/16.
//  Copyright © 2016 Game Design Initiative at Cornell. All rights reserved.
//

#include <cugl/math/CUMathBase.h>

/**
 * Returns the power of two greater than or equal to x
 * 
 * @param x		The original integer
 *
 * @return the power of two greater than or equal to x
 */
int nextPOT(int x) {
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return x + 1;
}

