#include "engine.h"
#include "rendertarget.h"

static struct glaretexture : rendertarget
{
    bool dorender()
    {
        extern void drawglare();
        drawglare();
        return true;
    }
} glaretex;

void cleanupglare()
{
    glaretex.cleanup(true);
}

VARFP(glaresize, 6, 8, 10, cleanupglare());
VARP(glare, 0, 0, 1);
VARP(blurglare, 0, 4, 7);
VARP(blurglaresigma, 1, 50, 200);

VAR(debugglare, 0, 0, 1);

void viewglaretex()
{
    if(!glare) return;
    glaretex.debug();
}

bool glaring = false;

void drawglaretex()
{
    if(!glare || renderpath==R_FIXEDFUNCTION) return;

    glaretex.render(1<<glaresize, 1<<glaresize, blurglare, blurglaresigma/100.0f);
}

FVARP(glarescale, 0, 1, 8);

void addglare()
{
    if(!glare || renderpath==R_FIXEDFUNCTION) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    SETSHADER(glare);

    glBindTexture(GL_TEXTURE_2D, glaretex.rendertex);

    setlocalparamf("glarescale", SHPARAM_PIXEL, 0, glarescale, glarescale, glarescale);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, 0);
    glTexCoord2f(1, 0); glVertex3f( 1, -1, 0);
    glTexCoord2f(0, 1); glVertex3f(-1,  1, 0);
    glTexCoord2f(1, 1); glVertex3f( 1,  1, 0);
    glEnd();

    glDisable(GL_BLEND);
}
     
