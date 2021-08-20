/**********
Copyright 2013 Dietmar Warning. All rights reserved.
Author:   2013 Dietmar Warning
**********/

#include "ngspice/ngspice.h"
#include "ngspice/cktdefs.h"
#include "bjtdefs.h"
#include "ngspice/trandefs.h"
#include "ngspice/sperror.h"
#include "ngspice/suffix.h"
#include "ngspice/cpdefs.h"


int
BJTsoaCheck(CKTcircuit *ckt, GENmodel *inModel)
{
    BJTmodel *model = (BJTmodel *) inModel;
    BJTinstance *here;
    double vbe, vbc, vce;    /* actual bjt voltages */
    double pow;
    double ic, ib;           /* actual bjt currents */
    int maxwarns;
    static int warns_vbe = 0, warns_vbc = 0, warns_vce = 0;
    static int warns_pow = 0, warns_ic = 0, warns_ib = 0;


    if (!ckt) {
        warns_vbe = 0;
        warns_vbc = 0;
        warns_vce = 0;
        warns_pow = 0;
        warns_ic = 0;
        warns_ib = 0;
        return OK;
    }

    maxwarns = ckt->CKTsoaMaxWarns;

    for (; model; model = BJTnextModel(model)) {

        for (here = BJTinstances(model); here; here = BJTnextInstance(here)) {

            vbe = fabs(ckt->CKTrhsOld[here->BJTbasePrimeNode] -
                ckt->CKTrhsOld[here->BJTemitPrimeNode]);
            vbc = fabs(ckt->CKTrhsOld[here->BJTbasePrimeNode] -
                ckt->CKTrhsOld[here->BJTcolPrimeNode]);
            vce = fabs(ckt->CKTrhsOld[here->BJTcolPrimeNode] -
                ckt->CKTrhsOld[here->BJTemitPrimeNode]);

            if (vbe > model->BJTvbeMax)
                if (warns_vbe < maxwarns) {
                    soa_printf(ckt, (GENinstance*)here,
                        "|Vbe|=%g has exceeded Vbe_max=%g\n",
                        vbe, model->BJTvbeMax);
                    warns_vbe++;
                }

            if (vbc > model->BJTvbcMax)
                if (warns_vbc < maxwarns) {
                    soa_printf(ckt, (GENinstance*)here,
                        "|Vbc|=%g has exceeded Vbc_max=%g\n",
                        vbc, model->BJTvbcMax);
                    warns_vbc++;
                }

            if (vce > model->BJTvceMax)
                if (warns_vce < maxwarns) {
                    soa_printf(ckt, (GENinstance*)here,
                        "|Vce|=%g has exceeded Vce_max=%g\n",
                        vce, model->BJTvceMax);
                    warns_vce++;
                }

            ic = fabs(*(ckt->CKTstate0 + here->BJTcc));
            if (ic > fabs(model->BJTicMax))
                if (warns_ic < maxwarns) {
                    soa_printf(ckt, (GENinstance*)here,
                        "Ic=%.4g A at Vce=%.4g V has exceeded Ic_max=%.4g A\n",
                        ic, vce, model->BJTicMax);
                    warns_ic++;
                }

            ib = fabs(*(ckt->CKTstate0 + here->BJTcb));
            if (ib > fabs(model->BJTibMax))
                if (warns_ib < maxwarns) {
                    soa_printf(ckt, (GENinstance*)here,
                        "Ib=%.4g A at Vbe=%.4g V has exceeded Ib_max=%.4g A\n",
                        ib, vbe, model->BJTibMax);
                    warns_ib++;
                }

            if (warns_pow < maxwarns) {
                pow = fabs(*(ckt->CKTstate0 + here->BJTcc) *
                    (*(ckt->CKTrhsOld + here->BJTcolNode) -
                        *(ckt->CKTrhsOld + here->BJTemitNode))
                );
                pow += fabs(*(ckt->CKTstate0 + here->BJTcb) *
                    (*(ckt->CKTrhsOld + here->BJTbaseNode) -
                        *(ckt->CKTrhsOld + here->BJTemitNode))
                );
                pow += fabs(*(ckt->CKTstate0 + here->BJTcdsub) *
                    (*(ckt->CKTrhsOld + here->BJTsubstConNode) -
                        *(ckt->CKTrhsOld + here->BJTsubstNode))
                );
                if ((ckt->CKTcurrentAnalysis & DOING_TRAN) && !(ckt->CKTmode &
                    MODETRANOP)) {
                    pow += *(ckt->CKTstate0 + here->BJTcqsub) *
                        fabs(*(ckt->CKTrhsOld + here->BJTsubstConNode) -
                            *(ckt->CKTrhsOld + here->BJTsubstNode));
                }
                pow *= here->BJTm;
                /* derating without self-heating, external temp and model tnom given */
                if (model->BJTrth0Given && model->BJTpowMaxGiven && model->BJTtnomGiven) {
                    double pow_max;
                    if (here->BJTtemp < model->BJTtnom)
                        pow_max = model->BJTpowMax;
                    else {
                        pow_max = model->BJTpowMax - (here->BJTtemp - model->BJTtnom) / model->BJTrth0;
                        pow_max = (pow_max > 0) ? pow_max : 0.;
                    }
                    if (pow > pow_max) {
                        soa_printf(ckt, (GENinstance*)here,
                            "Pow=%.4g W has exceeded Pow_max=%.4g W\n        at Vce=%.4g V, Ib=%.4g A, Ic=%.4g A, and Te=%.4g C\n",
                            pow, pow_max, vce, ib, ic, here->BJTtemp - CONSTCtoK);
                        warns_pow++;
                    }
                }
                /* no derating */
                else {
                    if (pow > model->BJTpowMax) {
                        soa_printf(ckt, (GENinstance*)here,
                            "Pow=%.4g W has exceeded Pow_max=%.4g W\n        at Vce=%.4g V, Ib=%.4g A, and Ic=%.4g A\n",
                            pow, model->BJTpowMax, vce, ib, ic);
                        warns_pow++;
                    }
                }
            }
        }

    }

    return OK;
}
