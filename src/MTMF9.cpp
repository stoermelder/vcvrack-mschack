#include "mscHack.hpp"

#define nCHANNELS 9

struct MTMF9_AMT_Knob : MSCH_Widget_Knob1 
{
    MTMF9_AMT_Knob()
    {
        set( DWRGB( 255, 255, 255 ), DWRGB( 255, 255, 255 ), 5.5f );
    }
};

struct MTMF9_Knob : MSCH_Widget_Knob1 
{
    MTMF9_Knob()
    {
        set( DWRGB( 255, 255, 255 ), DWRGB( 255, 255, 255 ), 9.5f );
    }
};

struct MTMF9_Knob_Snap : MSCH_Widget_Knob1 
{
    MTMF9_Knob_Snap()
    {
        Knob::snap = true;
        set( DWRGB( 180, 180, 180 ), DWRGB( 180, 180, 180 ), 9.5f );
    }
};

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct MTMF9 : Module
{
	enum ParamIds 
    {
        PARAM_FINE,
        PARAM_FINE_CV_AMT	= PARAM_FINE + nCHANNELS,
        nPARAMS			    = PARAM_FINE_CV_AMT + nCHANNELS
    };

	enum InputIds 
    {
		IN_VOCT,
		IN_FINE_CV	= IN_VOCT + nCHANNELS,
        nINPUTS 	= IN_FINE_CV + nCHANNELS
	};

	enum OutputIds 
    {
        OUT_POLY_VOCT,
		OUT_VOCT,
        nOUTPUTS	= OUT_VOCT + nCHANNELS,
	};

	enum LightIds 
    {
        ENUMS(PHASE_LIGHT, 3 * nCHANNELS),
        nLIGHTS
	};

    bool            m_bInitialized = false;

    // Contructor
	MTMF9()
    {
        char strVal[ 20 ] = {};

        config(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS);

        for( int i = 0; i < nCHANNELS; i++ )
        {
            snprintf( strVal, sizeof(strVal), "Ch %d Fine", i + 1 );
            configParam( PARAM_FINE + i, 0.0f, 1.0f, 0.5f, strVal );
            snprintf( strVal, sizeof(strVal), "Ch %d Fine CV Amount", i + 1 );
            configParam( PARAM_FINE_CV_AMT + i, 0.0f, 1.0f, 0.0f, strVal );

            configInput( IN_VOCT + i, string::f("Ch %d V/Oct", i + 1) );
            configInput( IN_FINE_CV + i, string::f("Ch %d Fine CV", i + 1) );
            configOutput( OUT_VOCT + i, string::f("Ch %d V/Oct", i + 1) );
        }

        configOutput( OUT_POLY_VOCT, "Poly V/Oct" );
    }

    // Overrides 
    void    JsonParams( bool bTo, json_t *root);
    void    process(const ProcessArgs &args) override;
    json_t* dataToJson() override;
    void    dataFromJson(json_t *rootJ) override;
    void    onReset() override;

    float   ProcessCV( int param, int lfocv, int amtparam, bool bLinearParam );
};

// dumb
MTMF9 g_MTMF9_Browser;

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------
struct MTMF9_Widget : ModuleWidget 
{

MTMF9_Widget( MTMF9 *module )
{
	int y;
    MTMF9 *pmod;

	//box.size = Vec( 15*12, 380);

    setModule(module);

    if( !module )
        pmod = &g_MTMF9_Browser;
    else
        pmod = module;

    setPanel(APP->window->loadSvg(asset::plugin( thePlugin, "res/MTMF9.svg")));

    //y = 24;

    //addInput(createInput<MyPortInTiny>( Vec( 7, y ), pmod, MTMF9::IN_VOCT_POLY ) );

    y = 54;

	for( int ch = 0; ch < nCHANNELS; ch++ )
	{
		// inputs
		addInput(createInput<MyPortInTiny>( Vec( 7, y ), pmod, MTMF9::IN_VOCT + ch ) );
		addInput(createInput<MyPortInTiny>( Vec( 70, y ), pmod, MTMF9::IN_FINE_CV + ch ) );

        // params
        addParam(createParam<MTMF9_Knob>( Vec( 28, y -5), pmod, MTMF9::PARAM_FINE + ch  ) );
        addParam(createParam<MTMF9_AMT_Knob>( Vec( 54, y-1 ), pmod, MTMF9::PARAM_FINE_CV_AMT + ch ) );

        // outputs
        addOutput(createOutput<MyPortOutTiny>( Vec( 90, y ), pmod, MTMF9::OUT_VOCT + ch ) );

        // lights
        addChild(createLight<SmallLight<RedGreenBlueLight>>(Vec(83, y + 1.5f), module, MTMF9::PHASE_LIGHT + (ch*3)));
        y += 30;
	}

    // poly outputs
    addOutput(createOutput<MyPortOutTiny>( Vec( 90, 322 ), pmod, MTMF9::OUT_POLY_VOCT ) );

	addChild(createWidget<ScrewSilver>(Vec(15, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createWidget<ScrewSilver>(Vec(15, 365))); 
	addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

    if( module )
    {
        module->m_bInitialized = true;
        module->onReset();
    }
}
};

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void MTMF9::JsonParams( bool bTo, json_t *root)
{
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *MTMF9::dataToJson()
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void MTMF9::dataFromJson( json_t *root )
{
    JsonParams( FROMJSON, root );

    if( !m_bInitialized )
        return;
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void MTMF9::onReset()
{
}

//-----------------------------------------------------
// Procedure:   ProcessCV
//
//-----------------------------------------------------
float MTMF9::ProcessCV( int param, int lfocv, int amtparam, bool bLinearParam )
{
    float lfo = 0.0f;

    if( inputs[ lfocv ].isConnected() )
        lfo = clamp( inputs[ lfocv ].getVoltage() / CV_MAXn5, -1.0f, 1.0f ) * ( params[ amtparam ].getValue() * params[ amtparam ].getValue() );

    if( bLinearParam )
        return clamp( params[ param ].getValue() + lfo, 0.0f, 1.0f );
    else
        return clamp( ( params[ param ].getValue() * params[ param ].getValue() ) + lfo, 0.0f, 1.0f );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
#define SEMI    ( 1.0f / 12.0f )

void MTMF9::process(const ProcessArgs &args)
{
    float voct, fine;
    int count = 0;
    int nPoly = 0;

	if( !m_bInitialized )
		return;

    if(inputs[IN_VOCT].isConnected())
    {
        if( inputs[IN_VOCT].getChannels() )
            nPoly = inputs[IN_VOCT].getChannels();
    }

	for( int ch = 0; ch < nCHANNELS; ch++ )
	{
        if(ch < nPoly)
        {
            fine = (ProcessCV(PARAM_FINE + ch, IN_FINE_CV + ch, PARAM_FINE_CV_AMT + ch, true) - 0.5f) * (SEMI * 2);
            voct = inputs[IN_VOCT].getPolyVoltage( ch ) + fine;

            outputs[OUT_VOCT + ch].setVoltage(voct);
            outputs[OUT_POLY_VOCT].setVoltage(voct, ch);
            lights[PHASE_LIGHT + 2 + (ch*3)].setBrightness(1.0f);
            count++;
        }
        else
        {
            if(inputs[IN_VOCT + ch].isConnected())
            {
                fine = (ProcessCV(PARAM_FINE + ch, IN_FINE_CV + ch, PARAM_FINE_CV_AMT + ch, true) - 0.5f) * (SEMI * 2);
                voct = inputs[IN_VOCT + ch].getNormalVoltage(0.0f) + fine;

                outputs[OUT_VOCT + ch].setVoltage(voct);
                outputs[OUT_POLY_VOCT].setVoltage(voct, ch);
                lights[PHASE_LIGHT + 2 + (ch*3)].setBrightness(1.0f);
                count = ch + 1;
            }
            else
            {
                outputs[OUT_VOCT + ch].setVoltage(0.0f);
                outputs[OUT_POLY_VOCT].setVoltage(0.0f, ch);
                lights[PHASE_LIGHT + 2 + (ch*3)].setBrightness(0.f);
            }
        }
	}

    outputs[ OUT_POLY_VOCT ].setChannels( count );
}

Model *modelMTMF9 = createModel<MTMF9, MTMF9_Widget>( "MTMF9" );
