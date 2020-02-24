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
// debugging.cpp - debugging utilities
#include <NTL/xdouble.h>
#include "debugging.h"
#include "norms.h"
#include "FHEContext.h"
#include "Ctxt.h"
#include "EncryptedArray.h"

NTL_CLIENT

FHESecKey* dbgKey = 0;
EncryptedArray* dbgEa = 0;
NTL::ZZX dbg_ptxt;
NTL::Vec<NTL::ZZ> ptxt_pwr; // powerful basis

// return the ratio between the real noise <sk,ct> and the estimated one
double realToEstimatedNoise(const Ctxt& ctxt, const FHESecKey& sk)
{
  xdouble noiseEst = ctxt.getNoiseBound();
  if (ctxt.isCKKS())
    noiseEst += ctxt.getRatFactor() * ctxt.getPtxtMag();
  xdouble actualNoise = embeddingLargestCoeff(ctxt, sk);

  return conv<double>(actualNoise/noiseEst);
}

double log2_realToEstimatedNoise(const Ctxt& ctxt, const FHESecKey& sk)
{
  xdouble noiseEst = ctxt.getNoiseBound();
  if (ctxt.isCKKS())
    noiseEst += ctxt.getRatFactor() * ctxt.getPtxtMag();
  xdouble actualNoise = embeddingLargestCoeff(ctxt, sk);

  return log(actualNoise/noiseEst)/log(2.0);
}

// check that real-to-estimated ratio is not too large, print warning otherwise
void checkNoise(const Ctxt& ctxt, const FHESecKey& sk, const std::string& msg, double thresh)
{
   double ratio;
   if ((ratio=realToEstimatedNoise(ctxt, sk)) > thresh) {
      std::cerr << "\n*** too much noise: " << msg << ": " << ratio << "\n";
   }
}

// Decrypt and find the l-infinity norm of the result in canonical embedding
xdouble embeddingLargestCoeff(const Ctxt& ctxt, const FHESecKey& sk) 
{
  const FHEcontext& context = ctxt.getContext();
  ZZX p, pp;
  sk.Decrypt(p, ctxt, pp);
  return embeddingLargestCoeff(pp, context.zMStar);
}


void decryptAndPrint(ostream& s, const Ctxt& ctxt, const FHESecKey& sk,
		     const EncryptedArray& ea, long flags)
{
  const FHEcontext& context = ctxt.getContext();
  vector<ZZX> ptxt;
  ZZX p, pp;
  sk.Decrypt(p, ctxt, pp);

  xdouble modulus = xexp(context.logOfProduct(ctxt.getPrimeSet()));
  xdouble actualNoise = embeddingLargestCoeff(pp, ctxt.getContext().zMStar);
  xdouble noiseEst = ctxt.getNoiseBound();
  if (ctxt.isCKKS())
    noiseEst += ctxt.getRatFactor() * ctxt.getPtxtMag();

  s << "plaintext space mod "<<ctxt.getPtxtSpace()
    << ", bitCapacity="<<ctxt.bitCapacity()
    << ", \n           |noise|=q*" << (actualNoise/modulus)
    << ", |noiseBound|=q*" << (noiseEst/modulus);
  if (ctxt.isCKKS()) {
    s << ", \n           ratFactor="<<ctxt.getRatFactor()
      << ", ptxtMag="<<ctxt.getPtxtMag()
      << ", realMag="<<(actualNoise/ ctxt.getRatFactor());
  }
  s << endl;

  if (flags & FLAG_PRINT_ZZX) {
    s << "   before mod-p reduction=";
    printZZX(s,pp) <<endl;
  }
  if (flags & FLAG_PRINT_POLY) {
    s << "   after mod-p reduction=";
    printZZX(s,p) <<endl;
  }
  if (flags & FLAG_PRINT_VEC) { // decode to a vector of ZZX
    ea.decode(ptxt, p);
    if (ea.getAlMod().getTag() == PA_zz_p_tag
	&& ctxt.getPtxtSpace() != ea.getAlMod().getPPowR()) {
      long g = GCD(ctxt.getPtxtSpace(), ea.getAlMod().getPPowR());
      for (long i=0; i<ea.size(); i++)
	PolyRed(ptxt[i], g, true);
    }
    s << "   decoded to ";
    if (deg(p) < 40) // just pring the whole thing
      s << ptxt << endl;
    else if (ptxt.size()==1) // a single slot
      printZZX(s, ptxt[0]) <<endl;
    else { // print first and last slots
      printZZX(s, ptxt[0],20) << "--";
      printZZX(s, ptxt[ptxt.size()-1], 20) <<endl;      
    }
  }
  else if (flags & FLAG_PRINT_DVEC) { // decode to a vector of doubles
    const EncryptedArrayCx& eacx = ea.getCx();
    vector<double> v;
    eacx.decrypt(ctxt, sk, v);
    printVec(s<<"           ", v,20)<<endl;
  }
  else if (flags & FLAG_PRINT_XVEC) { // decode to a vector of complex
    const EncryptedArrayCx& eacx = ea.getCx();
    vector<cx_double> v;
    eacx.decrypt(ctxt, sk, v);
    printVec(s<<"           ", v,20)<<endl;
  }
}

bool decryptAndCompare(const Ctxt& ctxt, const FHESecKey& sk,
		       const EncryptedArray& ea, const PlaintextArray& pa)
{
  PlaintextArray ppa(ea);
  ea.decrypt(ctxt, sk, ppa);

  return equals(ea, pa, ppa);
}

// Compute decryption with.without mod-q on a vector of ZZX'es,
// useful when debugging bootstrapping (after "raw mod-switch")
void rawDecrypt(ZZX& plaintxt, const std::vector<ZZX>& zzParts,
                const DoubleCRT& sKey, long q)
{
  const FHEcontext& context = sKey.getContext();

  // Set to zzParts[0] + sKey * zzParts[1] "over the integers"
  DoubleCRT ptxt = sKey;
  ptxt *= zzParts[1];
  ptxt += zzParts[0];

  // convert to coefficient representation
  ptxt.toPoly(plaintxt);

  if (q>1)
    PolyRed(plaintxt, q, false/*reduce to [-q/2,1/2]*/);
}

#include "powerful.h"
void CheckCtxt(const Ctxt& c, const char* label)
{
  cerr << "  "<<label 
       << ", log2(modulus/noise)=" << (-c.log_of_ratio()/log(2.0)) 
       << ", p^r=" << c.getPtxtSpace();

  if (dbgKey) {
    double ratio = log2_realToEstimatedNoise(c, *dbgKey);
    cerr << ", log2(noise/bound)=" << ratio;
    if (ratio > 0) cerr << " BAD-BOUND";
  }

  if (dbgKey && c.getContext().isBootstrappable()) {
    Ctxt c1(c);
    //c1.dropSmallAndSpecialPrimes();

    const FHEcontext& context = c1.getContext();
    const RecryptData& rcData = context.rcData;
    const PAlgebra& palg = context.zMStar;

    ZZX p, pp;
    dbgKey->Decrypt(p, c1, pp);
    Vec<ZZ> powerful;
    rcData.p2dConv->ZZXtoPowerful(powerful, pp);

    ZZ q;
    q = context.productOfPrimes(c1.getPrimeSet());
    vecRed(powerful, powerful, q, false);

    ZZX pp_alt;
    rcData.p2dConv->powerfulToZZX(pp_alt, powerful);

    xdouble max_coeff = conv<xdouble>(largestCoeff(pp));
    xdouble max_pwrfl = conv<xdouble>(largestCoeff(powerful));
    xdouble max_canon = embeddingLargestCoeff(pp_alt, palg);
    double ratio = log(max_pwrfl/max_canon)/log(2.0);

    //cerr << ", max_coeff=" << max_coeff;
    //cerr << ", max_pwrfl=" << max_pwrfl;
    //cerr << ", max_canon=" << max_canon;
    cerr << ", log2(max_pwrfl/max_canon)=" << ratio;
    if (ratio > 0) cerr << " BAD-BOUND";
  }
  cerr << endl;
}
