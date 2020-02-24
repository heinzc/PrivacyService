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

/* GTest_extractDigits.cpp - extracting digits.
 *   For a plaintext space modulo a prime-power $p^e$, extracting
 *   the base-$p$ representation of an encrypted values.
 */
#include <NTL/ZZ.h>
#include <EncryptedArray.h>
#include <polyEval.h>

#include "gtest/gtest.h"
#include "test_common.h"

#include "test_common.h"
#include "debugging.h"

namespace {

struct Parameters {
    Parameters(long p, long r, long m) :
        p(p),
        r(r),
        m(m)
    { if(p < 2) throw std::invalid_argument("p must be at least 2"); };
    long p; // Plaintext base
    long r; // Lifting
    long m; // The cyclotomic ring

    // Let googletest know how to print the Parameters
    friend std::ostream& operator<<(std::ostream& os, const Parameters& params) {
        return os << "{" <<
            "p=" << params.p << "," <<
            "r=" << params.r << "," <<
            "m=" << params.m <<
            "}";
    };
};

class GTest_extractDigits : public ::testing::TestWithParam<Parameters> {
    protected:
        long p;
        long r;
        long m;
        long p2r;
        long L;

        FHEcontext context;
        FHESecKey secretKey;
        const FHEPubKey& publicKey;

        // lifting value rOld requires some manipulation before being used.
        // Utility function for calculating this.
        static long correctLifting(long rOld, long p)
        {
            long r;
            double lBound = 30.0;
            long bound = floor(lBound/log2((double)p));
            if (r<2 || r>bound) r = bound;
            return r;
        };

        // Calculate how many levels we need
        static long calculateLevels(long r, long p)
        {
            long ll = NTL::NextPowerOfTwo(p);
            return 30*(r*ll*3 + 2);
        };

        GTest_extractDigits() :
            p(GetParam().p),
            r(correctLifting(GetParam().r, p)),
            m(GetParam().m ? GetParam().m : p + 1), // FindM(/*secparam=*/80, L, /*c=*/4, p, /*d=*/1, 0, m);
            p2r(NTL::power_long(p,r)),
            L(calculateLevels(r, p)),
            context(m, p, r),
            secretKey((buildModChain(context, L, /*c=*/4), context)),
            publicKey(secretKey)
        {
        }

        virtual void SetUp() override
        {
            // You can put setup operations here or in the constructor
            setDryRun(helib_test::dry);
            if (!helib_test::noPrint) {
                if (helib_test::dry) std::cout << "dry run: ";
                std::cout << "m=" << m << ", p=" << p << ", r=" << r << ", L=" << L <<  std::endl;
            }

            secretKey.GenSecKey(); // A +-1/0 secret key
            addSome1DMatrices(secretKey); // compute key-switching matrices that we need
            // On legacy test is debug, but used verbose for consistency with other tests 
            if (helib_test::verbose) {
              dbgKey = &secretKey; // debugging key and ea
              dbgEa = (EncryptedArray*) context.ea;
            }

#ifdef DEBUG_PRINTOUT
            dbgKey = &secretKey;
            dbgEa = const_cast<EncryptedArray*>(context.ea);
#endif // DEBUG_PRINTOUT
        };

        virtual void TearDown() override
        {
            cleanupGlobals();
        }
};

TEST_P(GTest_extractDigits, correctly_extracts_digits)
{
    EncryptedArray ea(context);
    std::vector<long> v;
    std::vector<long> pDigits;
    ea.random(v); // random values in the slots

    const FHEPubKey& publicKey = secretKey;

    Ctxt c(publicKey);
    ea.encrypt(c, publicKey, v);
    ea.decrypt(c, secretKey, pDigits);
    if (ea.size()<=20 && !helib_test::noPrint)
        std::cout << "plaintext=" << pDigits << std::endl;

    if (!helib_test::noPrint)
        std::cout << "extracting " << r << " digits..." << std::flush;
    std::vector<Ctxt> digits;
    extractDigits(digits, c);
    if (!helib_test::noPrint)
        std::cout << " done\n" << std::flush;

    std::vector<long> tmp = v;
    long pp = p2r;
    for (long i=0; i<(long)digits.size(); i++) {
        if (!digits[i].isCorrect()) {
            CheckCtxt(digits[i], "");
            FAIL() << " potential decryption error for " << i <<"th digit ";
        }
        ea.decrypt(digits[i], secretKey, pDigits);
        if (ea.size()<=20 && !helib_test::noPrint)
            std::cout << i << "th digit=" << pDigits << std::endl;

        // extract the next digit from the plaintext, compare to pDigits
        for (long j=0; j<(long)v.size(); j++) {
            long digit = tmp[j] % p;
            if (digit > p/2) digit -= p;
            else if (digit < -p/2) digit += p;

            EXPECT_EQ((pDigits[j] - digit) % pp, 0)
                << " error: v[" << j << "]=" << v[j]
                << " but " << i << "th digit comes " << pDigits[j]
                << " rather than " << digit << std::endl << std::endl;
            tmp[j] -= digit;
            tmp[j] /= p;
        }
        pp /= p;
    }
}

INSTANTIATE_TEST_SUITE_P(various_plaintext_bases, GTest_extractDigits, ::testing::Values(
            //SLOW
            Parameters(5, 0, 2047)
            //FAST
            //Parameters(5, 0, 91)
            ));

} // anonymous namespace
