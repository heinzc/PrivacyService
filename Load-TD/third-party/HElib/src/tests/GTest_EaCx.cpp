/* Copyright (C) 2012-2017 IBM Corp.
 * This program is Licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. See accompanying LICENSE file.
 */
#include <NTL/ZZ.h>
#include "norms.h"
#include "EncryptedArray.h"

#ifdef DEBUG_PRINTOUT
#include "debugging.h"
#endif

#include "gtest/gtest.h"
#include "test_common.h"

namespace {
struct Parameters {
    const long m;
    const long r;

    Parameters(long m, long r) :
        m(m),
        r(r)
    {};

    friend std::ostream& operator<<(std::ostream& os, const Parameters& params)
    {
        return os << "{" <<
            "m=" << params.m << "," <<
            "r=" << params.r << 
            "}";
    };
};

class GTest_EaCx : public ::testing::TestWithParam<Parameters> {
    protected:

        const long m;
        const long r;
        FHEcontext context;
        const EncryptedArrayCx& eacx;

        GTest_EaCx() :
            m(GetParam().m),
            r(GetParam().r),
            context(m, /*p=*/-1, r),
            eacx((buildModChain(context, 5, 2), context.ea->getCx()))
        {}

        virtual void SetUp() override 
        {
            if (!helib_test::noPrint) {
                std::vector<long> f;
                factorize(f,m);
                std::cout << "r="<<r<<", factoring "<<m<<" gives [";
                for (unsigned long i=0; i<f.size(); i++)
                    std::cout << f[i] << " ";
                std::cout << "]\n";

                eacx.getPAlgebra().printout();

#ifdef DEBUG_PRINTOUT
                std::vector<cx_double> vc1;
                eacx.random(vc1);
                std::cout << "random complex vc1=";
                printVec(std::cout,vc1,8)<<std::endl;

                std::vector<double> vd;
                eacx.random(vd);
                std::cout << "random real vd=";
                printVec(std::cout,vd,8)<<std::endl;
#endif
            }
        }

        virtual void TearDown() override
        {
#ifdef DEBUG_PRINTOUT
            cleanupGlobals();
#endif
        }
};

TEST_P(GTest_EaCx, encoding_works_correctly)
{
    std::vector<double> vl;
    eacx.random(vl);
    vl[1] = -1; // ensure that this is not the zero vector
#ifdef DEBUG_PRINTOUT
    std::cout << "random int v=";
    printVec(std::cout,vl,8)<<std::endl;
#endif

    zzX poly;
    double factor = eacx.encode(poly, vl, 1.0);
    if (!helib_test::noPrint) {
      NTL::ZZX poly2;
      convert(poly2, poly);
      std::cout << "  encoded into a degree-"<<NTL::deg(poly2)<<" polynomial\n";
    }

    std::vector<double> vd2;
    eacx.decode(vd2, poly, factor);
#ifdef DEBUG_PRINTOUT
    std::cout << "  decoded into vd2=";
    printVec(std::cout,vd2,8)<<std::endl;
#endif
    EXPECT_EQ(lsize(vl), lsize(vd2));

    double maxDiff = 0.0;
    for (long i=0; i<lsize(vl); i++) {
        double diffAbs = std::abs(vl[i]-vd2[i]);
        if (diffAbs > maxDiff)
            maxDiff = diffAbs;
    }
    EXPECT_LE(maxDiff, 0.1) <<
        "max |v-vd2|_{infty}=" << maxDiff;

}

INSTANTIATE_TEST_SUITE_P(small_parameters, GTest_EaCx, ::testing::Values(
            Parameters(16, 8)
            ));

} // namespace