/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Gordon Jacobs
**********/
/*
 */

#include "ngspice/ngspice.h"
#include "ngspice/cktdefs.h"
#include "cswdefs.h"
#include "ngspice/sperror.h"
#include "ngspice/suffix.h"


int
CSWacLoad(GENmodel *inModel, CKTcircuit *ckt)

        /* load the current values into the 
         * sparse matrix previously provided 
         * during AC analysis
         */
{
    CSWmodel *model = (CSWmodel*)inModel;
    CSWinstance *here;
    double g_now;
    int current_state;

    /*  loop through all the switch models */
    for( ; model; model = CSWnextModel(model))

        /* loop through all the instances of the model */
        for (here = CSWinstances(model); here;
                here=CSWnextInstance(here)) {

            current_state = (int) ckt->CKTstates[0][here->CSWstate + 0];

            g_now = current_state?(model->CSWonConduct):(model->CSWoffConduct);

            *(here->CSWposPosPtr) += g_now;
            *(here->CSWposNegPtr) -= g_now;
            *(here->CSWnegPosPtr) -= g_now;
            *(here->CSWnegNegPtr) += g_now;
        }
    return OK;
}
