/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#ifndef FRONTENDAPPLICATIONBASE_HPP
#define FRONTENDAPPLICATIONBASE_HPP

#include <mvp/MVPApplication.hpp>
#include <gui/model/Model.hpp>

class FrontendHeap;

class FrontendApplicationBase : public touchgfx::MVPApplication
{
public:
    FrontendApplicationBase(Model& m, FrontendHeap& heap);
    virtual ~FrontendApplicationBase() { }

    // Menu
    void gotoMenuScreenNoTransition();


    // Splash
    void gotoSplashScreenNoTransition();


    // PopUp
    void gotoPopUpScreenCoverTransitionNorth();

protected:
    touchgfx::Callback<FrontendApplicationBase> transitionCallback;
    FrontendHeap& frontendHeap;
    Model& model;

    // Menu
    void gotoMenuScreenNoTransitionImpl();


    // Splash
    void gotoSplashScreenNoTransitionImpl();


    // PopUp
    void gotoPopUpScreenCoverTransitionNorthImpl();
};

#endif // FRONTENDAPPLICATIONBASE_HPP