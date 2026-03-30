#include "mscHack.hpp"

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Tremolio9 : Module 
{
#define nCHANNELS MAX_ENVELOPE_CHANNELS
#define nWAVESETS 4
#define nTIMESETS 6
#define nMAXFREQ   50 // Hz
#define nMAXSPEED  2 // Hz

#define WAVE_BUFFER_LEN ( 192000 / 20 ) // (9600) based on quality for 20Hz at max sample rate 192000

	enum ParamIds 
    {
        PARAM_BAND,
        PARAM_MAX_FREQ,
        PARAM_SPEED             = PARAM_MAX_FREQ + nCHANNELS,
        nPARAMS                 = PARAM_SPEED + nCHANNELS
    };

	enum InputIds 
    {
        INPUT_CH_TRIG,
        nINPUTS                 = INPUT_CH_TRIG + nCHANNELS
	};

	enum OutputIds 
    {
        OUTPUT_CV,
        nOUTPUTS                = OUTPUT_CV + nCHANNELS
	};

	enum LightIds 
    {
        nLIGHTS
	};

    enum DataViewIds
    {
        DATA_A = 0,
        DATA_B = 1
    };

    bool                m_bInitialized = false;
    bool                m_bDataLoaded = false;
    std::shared_ptr<Widget_EnvelopeEdit::EditData> m_EditDataFreq;
    std::shared_ptr<Widget_EnvelopeEdit::EditData> m_EditDataAmount;

    // Contructor
	Tremolio9()
    {
        m_EditDataFreq = std::make_shared<Widget_EnvelopeEdit::EditData>();
        m_EditDataAmount = std::make_shared<Widget_EnvelopeEdit::EditData>();

        config(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS);

        configParam( PARAM_BAND, 0.0, 0.8, 0.333, "Rubber Band Edit" );
        configParam( PARAM_MAX_FREQ + 0, 0.0, 1.0, 0.5, "Ch1 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 1, 0.0, 1.0, 0.5, "Ch2 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 2, 0.0, 1.0, 0.5, "Ch3 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 3, 0.0, 1.0, 0.5, "Ch4 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 4, 0.0, 1.0, 0.5, "Ch5 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 5, 0.0, 1.0, 0.5, "Ch6 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 6, 0.0, 1.0, 0.5, "Ch7 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 7, 0.0, 1.0, 0.5, "Ch8 Max Vibrato Frequency" );
        configParam( PARAM_MAX_FREQ + 8, 0.0, 1.0, 0.5, "Ch9 Max Vibrato Frequency" );

        configParam( PARAM_SPEED + 0, 0.0, 1.0, 0.5, "Ch1 Speed" );
        configParam( PARAM_SPEED + 1, 0.0, 1.0, 0.5, "Ch2 Speed" );
        configParam( PARAM_SPEED + 2, 0.0, 1.0, 0.5, "Ch3 Speed" );
        configParam( PARAM_SPEED + 3, 0.0, 1.0, 0.5, "Ch4 Speed" );
        configParam( PARAM_SPEED + 4, 0.0, 1.0, 0.5, "Ch5 Speed" );
        configParam( PARAM_SPEED + 5, 0.0, 1.0, 0.5, "Ch6 Speed" );
        configParam( PARAM_SPEED + 6, 0.0, 1.0, 0.5, "Ch7 Speed" );
        configParam( PARAM_SPEED + 7, 0.0, 1.0, 0.5, "Ch8 Speed" );
        configParam( PARAM_SPEED + 8, 0.0, 1.0, 0.5, "Ch9 Speed" );

        configInput( INPUT_CH_TRIG + 0, "Channel 1 Trigger" );
        configInput( INPUT_CH_TRIG + 1, "Channel 2 Trigger" );
        configInput( INPUT_CH_TRIG + 2, "Channel 3 Trigger" );
        configInput( INPUT_CH_TRIG + 3, "Channel 4 Trigger" );
        configInput( INPUT_CH_TRIG + 4, "Channel 5 Trigger" );
        configInput( INPUT_CH_TRIG + 5, "Channel 6 Trigger" );
        configInput( INPUT_CH_TRIG + 6, "Channel 7 Trigger" );
        configInput( INPUT_CH_TRIG + 7, "Channel 8 Trigger" );
        configInput( INPUT_CH_TRIG + 8, "Channel 9 Trigger" );
        configOutput( OUTPUT_CV + 0, "Channel 1 CV" );
        configOutput( OUTPUT_CV + 1, "Channel 2 CV" );
        configOutput( OUTPUT_CV + 2, "Channel 3 CV" );
        configOutput( OUTPUT_CV + 3, "Channel 4 CV" );
        configOutput( OUTPUT_CV + 4, "Channel 5 CV" );
        configOutput( OUTPUT_CV + 5, "Channel 6 CV" );
        configOutput( OUTPUT_CV + 6, "Channel 7 CV" );
        configOutput( OUTPUT_CV + 7, "Channel 8 CV" );
        configOutput( OUTPUT_CV + 8, "Channel 9 CV" );
    }

    float           m_BufferWave[ WAVE_BUFFER_LEN ] = {};

    // channel triggers
    dsp::SchmittTrigger      m_SchTrigChTrig[ nCHANNELS ] ={};

    int                 m_CurrentChannel = 0;
    int                 m_GraphFreqData[ nCHANNELS ][ ENVELOPE_HANDLES ] = {};
    int                 m_GraphAmountData[ nCHANNELS ][ ENVELOPE_HANDLES ] = {};

    int                 m_DataView = DATA_A;

    bool                m_bTrig[ nCHANNELS ] = {};

    int                 m_waveSet = 0;
    bool                m_bCpy = false;

    float               m_phase[ nCHANNELS ] = {};
    float               m_speedphase[ nCHANNELS ] = {};

    Widget_EnvelopeEdit *m_pEnvelope = NULL;
    MyLEDButtonStrip    *m_pButtonChSelect = NULL;

    MyLEDButtonStrip    *m_pButtonEditSelect = NULL;

    MyLEDButton         *m_pButtonTrig[ nCHANNELS ] = {};
    MyLEDButton         *m_pButtonWaveSetBck = NULL;
    MyLEDButton         *m_pButtonWaveSetFwd = NULL;

    MyLEDButton         *m_pButtonDraw = NULL;
    MyLEDButton         *m_pButtonCopy = NULL;
    MyLEDButton         *m_pButtonRand = NULL;
    MyLEDButton         *m_pButtonInvert = NULL;
    MyLEDButton         *m_pButtonSmooth = NULL;

    ParamWidget         *m_pParamFreqKnob[ nCHANNELS ] = {};
    ParamWidget         *m_pParamSpeedKnob[ nCHANNELS ] = {};

    Label               *m_pTextLabel = NULL;
    Label               *m_pTextLabelFreq = NULL;
    Label               *m_pTextLabelSpeed = NULL;

    int                 m_BeatCount = 0;

    float               m_WaveRat;

    std::shared_ptr<Widget_EnvelopeEdit::EditData> GetViewEditData( int idata )
    {
        if( idata == DATA_B )
            return m_EditDataAmount;

        return m_EditDataFreq;
    }

    EnvelopeData*       GetViewData( int idata )
    {
        return GetViewEditData( idata )->m_EnvData;
    }

    void    ApplyGraphDataToEditData( void )
    {
        int ch;

        m_EditDataFreq->setDataAll( (int*)m_GraphFreqData );
        m_EditDataAmount->setDataAll( (int*)m_GraphAmountData );

        // Shared EditData::setDataAll() does not recalc lines, so loaded data must do it here.
        for( ch = 0; ch < nCHANNELS; ch++ )
        {
            m_EditDataFreq->m_EnvData[ ch ].recalcLine( -1 );
            m_EditDataAmount->m_EnvData[ ch ].recalcLine( -1 );
        }
    }

    //-----------------------------------------------------
    // Band_Knob
    //-----------------------------------------------------
    struct Band_Knob : Knob_Yellow2_26
    {
        Tremolio9 *mymodule;
        int param;

        void onChange( const event::Change &e ) override 
        {
            ParamQuantity* paramQuantity = getParamQuantity();
            mymodule = (Tremolio9*)paramQuantity->module;

            if(mymodule)
            {
                mymodule->m_EditDataFreq->m_fband = paramQuantity->getValue();
                mymodule->m_EditDataAmount->m_fband = paramQuantity->getValue();
            }

		    RoundKnob::onChange( e );
	    }
    };

    struct MaxFreq_Knob : Knob_Yellow2_26
    {
        char strVal[ 10 ] = {};
        Tremolio9 *mymodule;
        int param;

        void onChange( const event::Change &e ) override 
        {
            ParamQuantity* paramQuantity = getParamQuantity();
            mymodule = (Tremolio9*)paramQuantity->module;

            if(mymodule)
            {
                mymodule->m_EditDataFreq->m_fband = paramQuantity->getValue();
                mymodule->m_EditDataAmount->m_fband = paramQuantity->getValue();

                snprintf( strVal, sizeof(strVal), "[%.1fHz]", paramQuantity->getValue() * nMAXFREQ );
                mymodule->m_pTextLabelFreq->text = strVal;
            }

            RoundKnob::onChange( e );
        }
    };

    struct Speed_Knob : Knob_Yellow2_26
    {
        char strVal[ 10 ] = {};
        Tremolio9 *mymodule;
        int param;

        void onChange( const event::Change &e ) override 
        {
            ParamQuantity* paramQuantity = getParamQuantity();
            mymodule = (Tremolio9*)paramQuantity->module;

            if(mymodule)
            {
                snprintf( strVal, sizeof(strVal), "[%.1fHz]", paramQuantity->getValue() * nMAXSPEED );
                mymodule->m_pTextLabelSpeed->text = strVal;
            } 

            RoundKnob::onChange( e );
        }
    };

    void    ChangeChannel( int ch );
    void    ChangeDataView( int idata );
    void    BuildWaves( void );

    // Overrides 
    void    JsonParams( bool bTo, json_t *root);
    void    process(const ProcessArgs &args) override;
    json_t* dataToJson() override;
    void    dataFromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
    void    onSampleRateChange() override;
};

Tremolio9 Tremolio9Browser;

//-----------------------------------------------------
// Tremolio9_EnvelopeEditCALLBACK
//-----------------------------------------------------
void Tremolio9_EnvelopeEditCALLBACK ( void *pClass, float val )
{
    char strVal[ 10 ] = {};

    Tremolio9 *mymodule;
    mymodule = (Tremolio9*)pClass;

    if( !pClass )
        return;

    if( mymodule->m_DataView == Tremolio9::DATA_A )
        snprintf( strVal, sizeof(strVal), "[%.1fHz]", val * nMAXFREQ * mymodule->params[ Tremolio9::PARAM_MAX_FREQ + mymodule->m_CurrentChannel ].getValue() );
    else
        snprintf( strVal, sizeof(strVal), "[%.3fV]", val * 10.0f );

    mymodule->m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// Procedure:   SynthEdit_WaveSmooth
//-----------------------------------------------------
void Tremolio9_WaveSmooth( void *pClass, int id, bool bOn )
{
    Tremolio9 *m;
    m = (Tremolio9*)pClass;

    m->GetViewEditData( m->m_DataView )->smoothWave( m->m_CurrentChannel, 0.25f );
}

//-----------------------------------------------------
// Tremolio9_DrawMode
//-----------------------------------------------------
void Tremolio9_DrawMode( void *pClass, int id, bool bOn ) 
{
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;
    mymodule->m_EditDataFreq->m_bDraw = bOn;
    mymodule->m_EditDataAmount->m_bDraw = bOn;
}

//-----------------------------------------------------
// Procedure:   Tremolio9_ChSelect
//-----------------------------------------------------
void Tremolio9_ChSelect( void *pClass, int id, int nbutton, bool bOn )
{
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;

    mymodule->ChangeChannel( nbutton );
}

//-----------------------------------------------------
// Procedure:   Tremolio9_EditSelect
//-----------------------------------------------------
void Tremolio9_EditSelect( void *pClass, int id, int nbutton, bool bOn )
{
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;

    //if(bOn)
    //{
        mymodule->ChangeDataView( nbutton );
    //}
}

//-----------------------------------------------------
// Procedure:   Tremolio9_WaveSet
//-----------------------------------------------------
void Tremolio9_WaveSet( void *pClass, int id, bool bOn )
{
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;

    if( id == 0 )
    {
        if( ++mymodule->m_waveSet >= EnvelopeData::nPRESETS )
            mymodule->m_waveSet = 0;
    }
    else
    {
        if( --mymodule->m_waveSet < 0 )
            mymodule->m_waveSet = EnvelopeData::nPRESETS - 1;
    }

    mymodule->GetViewData( mymodule->m_DataView )[ mymodule->m_CurrentChannel ].Preset( mymodule->m_waveSet );
}

//-----------------------------------------------------
// Procedure:   Tremolio9_WaveInvert
//-----------------------------------------------------
void Tremolio9_WaveInvert( void *pClass, int id, bool bOn )
{
    int i;
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;

    EnvelopeData *pData = mymodule->GetViewData( mymodule->m_DataView );

    for( i = 0; i < ENVELOPE_HANDLES; i++ )
        pData[ mymodule->m_CurrentChannel ].setVal( i, 1.0f - pData[ mymodule->m_CurrentChannel ].m_HandleVal[ i ] );
}

//-----------------------------------------------------
// Procedure:   Tremolio9_WaveRand
//-----------------------------------------------------
void Tremolio9_WaveRand( void *pClass, int id, bool bOn )
{
    int i;
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;

    EnvelopeData *pData = mymodule->GetViewData( mymodule->m_DataView );

    for( i = 0; i < ENVELOPE_HANDLES; i++ )
        pData[ mymodule->m_CurrentChannel ].setVal( i, random::uniform() );
}

//-----------------------------------------------------
// Procedure:   Tremolio9_WaveCopy
//-----------------------------------------------------
void Tremolio9_WaveCopy( void *pClass, int id, bool bOn )
{
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;

    mymodule->m_bCpy = bOn;
}

//-----------------------------------------------------
// Procedure:   Tremolio9_Trig
//-----------------------------------------------------
void Tremolio9_Trig( void *pClass, int id, bool bOn )
{
    Tremolio9 *mymodule;

    if( !pClass )
        return;

    mymodule = (Tremolio9*)pClass;

    // global reset
    if( id == nCHANNELS )
    {
        // turn on all trigs
        for( int i = 0; i < nCHANNELS; i++ )
        {
            mymodule->m_pButtonTrig[ i ]->Set( bOn );
            mymodule->m_bTrig[ i ] = true;
        }
    }
    else
    {
        mymodule->m_bTrig[ id ] = true;
    }
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Tremolio9_Widget : ModuleWidget 
{

Tremolio9_Widget( Tremolio9 *module ) 
{
    Tremolio9 *pmod;
    int ch, x=0, y=0;

	//box.size = Vec( 15*36, 380);

    setModule(module);

    if( !module )
        pmod = &Tremolio9Browser;
    else
        pmod = module;

    //box.size = Vec( 15*21, 380);
    setPanel(APP->window->loadSvg(asset::plugin( thePlugin, "res/Tremolio9.svg")));

    addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(createWidget<ScrewSilver>(Vec(15, 365))); 
    addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

    // invert
    pmod->m_pButtonInvert = new MyLEDButton( 95, 23, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, Tremolio9_WaveInvert );
	addChild( pmod->m_pButtonInvert );

    // smooth
    pmod->m_pButtonSmooth = new MyLEDButton( 125, 23, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, Tremolio9_WaveSmooth );
    addChild( pmod->m_pButtonSmooth );

    // envelope editor
    pmod->m_pEnvelope = new Widget_EnvelopeEdit( 47, 42, 416, 192, 7, pmod->GetViewEditData( pmod->m_DataView ), module, Tremolio9_EnvelopeEditCALLBACK, nCHANNELS );
	addChild( pmod->m_pEnvelope );

    // envelope select buttons
    pmod->m_pButtonChSelect = new MyLEDButtonStrip( 210, 23, 11, 11, 3, 8.0, nCHANNELS, false, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, Tremolio9_ChSelect );
    addChild( pmod->m_pButtonChSelect );

    pmod->m_pTextLabel = new Label();
    pmod->m_pTextLabel->box.pos = Vec( 450, 10 );
    pmod->m_pTextLabel->text = "----";
	addChild( pmod->m_pTextLabel );

    // wave set buttons
    x = 364;
    y = 23;
    pmod->m_pButtonWaveSetBck = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, Tremolio9_WaveSet );
	addChild( pmod->m_pButtonWaveSetBck );

    pmod->m_pButtonWaveSetFwd = new MyLEDButton( x + 12, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 1, module, Tremolio9_WaveSet );
	addChild( pmod->m_pButtonWaveSetFwd );

    x = 405;

    // random
    pmod->m_pButtonRand = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, Tremolio9_WaveRand );
	addChild( pmod->m_pButtonRand );

    x += 25;

    // copy
    pmod->m_pButtonCopy = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_SWITCH, 0, module, Tremolio9_WaveCopy );
	addChild( pmod->m_pButtonCopy );

    // wave edit type select (freq or amount)
    pmod->m_pButtonEditSelect = new MyLEDButtonStrip( 117, 256, 11, 11, 8, 8.0, 2, true, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, Tremolio9_EditSelect );
    addChild( pmod->m_pButtonEditSelect );

    // draw mode
    pmod->m_pButtonDraw = new MyLEDButton( 48, 237, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 128, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, Tremolio9_DrawMode );
	addChild( pmod->m_pButtonDraw );

    // band knob
    addParam(createParam<Tremolio9::Band_Knob>( Vec( 59.5 , 280 ), module, Tremolio9::PARAM_BAND ) );

    // inputs, outputs
    x=21;
    y=53;
    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        addInput(createInput<MyPortInSmall>( Vec( x, y ), module, Tremolio9::INPUT_CH_TRIG + ch ) );

        // trig button
        pmod->m_pButtonTrig[ ch ] = new MyLEDButton( x -16, y + 2, 14, 14, 11.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, ch, module, Tremolio9_Trig );
	    addChild( pmod->m_pButtonTrig[ ch ] );

        // out cv
        addOutput(createOutput<MyPortOutSmall>( Vec( 516, y ), module, Tremolio9::OUTPUT_CV + ch ) );

        y += 28;

        // max freq knob m_pParamFreqKnob
        pmod->m_pParamFreqKnob[ ch ] = createParam<Tremolio9::MaxFreq_Knob>( Vec( 182 , 316 ), module, Tremolio9::PARAM_MAX_FREQ + ch );
        addParam( pmod->m_pParamFreqKnob[ ch ] );

        pmod->m_pParamFreqKnob[ ch ]->hide();

        // speed knob
        pmod->m_pParamSpeedKnob[ ch ] = createParam<Tremolio9::Speed_Knob>( Vec( 258 , 316 ), module, Tremolio9::PARAM_SPEED + ch );
        addParam( pmod->m_pParamSpeedKnob[ ch ] );

        pmod->m_pParamSpeedKnob[ ch ]->hide();

        // set the editable envelope parameters, these are internal and not user editable
        pmod->m_EditDataFreq->m_EnvData[ ch ].m_Range = EnvelopeData::RANGE_0to1;
        pmod->m_EditDataAmount->m_EnvData[ ch ].m_Range = EnvelopeData::RANGE_0to1;
        pmod->m_EditDataFreq->m_EnvData[ ch ].setMode( EnvelopeData::MODE_ONESHOT );
        pmod->m_EditDataAmount->m_EnvData[ ch ].setMode( EnvelopeData::MODE_ONESHOT );
    }

    pmod->m_pParamFreqKnob[ 0 ]->show();
    pmod->m_pParamSpeedKnob[ 0 ]->show();

    pmod->m_pTextLabelFreq = new Label();
    pmod->m_pTextLabelFreq->box.pos = Vec( 163, 348 );
    pmod->m_pTextLabelFreq->text = "25Hz";
    addChild( pmod->m_pTextLabelFreq );

    pmod->m_pTextLabelSpeed = new Label();
    pmod->m_pTextLabelSpeed->box.pos = Vec( 241, 348 );
    pmod->m_pTextLabelSpeed->text = "250Hz";
    addChild( pmod->m_pTextLabelSpeed );

    if( module )
    {
        module->onSampleRateChange();
        module->BuildWaves();
        module->m_pEnvelope->m_EditData = module->GetViewEditData( module->m_DataView );
        module->m_bInitialized = true;

        if( module->m_bDataLoaded )
        {
            module->ApplyGraphDataToEditData();
            module->ChangeChannel( 0 );
        }
        else
        {
            module->onReset();
        }
    }
}
};

//-----------------------------------------------------
// Procedure:   initialize
//
//-----------------------------------------------------
//#define DEG2RAD( x ) ( ( x ) * ( 3.14159f / 180.0f ) )
void Tremolio9::BuildWaves( void )
{
    int i;
    float finc, pos;

    finc = 360.0 / WAVE_BUFFER_LEN;
    pos = 0;

    // create sin wave
    for( i = 0; i < WAVE_BUFFER_LEN; i++ )
    {
        m_BufferWave[ i ] = sin( DEG2RAD( pos ) );
        pos += finc;
    }
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void Tremolio9::JsonParams( bool bTo, json_t *root) 
{
    JsonDataInt( bTo, "m_GraphFreqData", root, (int*)m_GraphFreqData, nCHANNELS * ENVELOPE_HANDLES );
    JsonDataInt( bTo, "m_GraphAmountData", root, (int*)m_GraphAmountData, nCHANNELS * ENVELOPE_HANDLES );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Tremolio9::dataToJson() 
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    if( m_bInitialized )
    {
        m_EditDataFreq->getDataAll( (int*)m_GraphFreqData );
        m_EditDataAmount->getDataAll( (int*)m_GraphAmountData );
    }

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void Tremolio9::dataFromJson( json_t *root ) 
{
    JsonParams( FROMJSON, root );

    m_bDataLoaded = true;

    if( !m_bInitialized )
        return;

    ApplyGraphDataToEditData();

    ChangeChannel( 0 );
}

//-----------------------------------------------------
// Procedure:   onSampleRateChange
//
//-----------------------------------------------------
void Tremolio9::onSampleRateChange()
{
    m_WaveRat = (float)(WAVE_BUFFER_LEN-1) / APP->engine->getSampleRate();
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void Tremolio9::onReset()
{
    m_bDataLoaded = false;

    memset( m_GraphFreqData, 0, sizeof( m_GraphFreqData ) );
    memset( m_GraphAmountData, 0, sizeof( m_GraphAmountData ) );
    memset( m_phase, 0, sizeof( m_phase ) );
    memset( m_speedphase, 0, sizeof( m_speedphase ) );

    ApplyGraphDataToEditData();

    ChangeChannel( 0 );
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void Tremolio9::onRandomize()
{
    int ch, i;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        for(i = 0; i < ENVELOPE_HANDLES; i++)
        {
            m_EditDataFreq->m_EnvData[ ch ].setVal( i, random::uniform() );
            m_EditDataAmount->m_EnvData[ ch ].setVal( i, random::uniform() );
        }
    }
}

//-----------------------------------------------------
// Procedure:   ChangeChannel
//
//-----------------------------------------------------
void Tremolio9::ChangeChannel( int ch )
{
    char strVal[ 10 ] = {};
    int i;

    if( ch < 0 || ch >= nCHANNELS )
        return;

    if( m_bCpy )
    {
        EnvelopeData *pData = GetViewData( m_DataView );

        m_bCpy = false;
        m_pButtonCopy->Set( false );

        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            pData[ ch ].setVal( i, pData[ m_CurrentChannel ].m_HandleVal[ i ] );
        }
    }

    m_pParamFreqKnob[ m_CurrentChannel ]->hide();
    m_pParamSpeedKnob[ m_CurrentChannel ]->hide();
    m_pParamFreqKnob[ ch ]->show();
    m_pParamSpeedKnob[ ch ]->show();

    snprintf( strVal, sizeof(strVal), "[%.1fHz]", params[ Tremolio9::PARAM_MAX_FREQ + ch ].getValue() * nMAXFREQ );
    m_pTextLabelFreq->text = strVal;

    snprintf( strVal, sizeof(strVal), "[%.1fHz]", params[ Tremolio9::PARAM_SPEED + ch ].getValue() * nMAXSPEED );
    m_pTextLabelSpeed->text = strVal;

    m_CurrentChannel = ch;
    m_pButtonChSelect->Set( ch, true );
    m_pEnvelope->setView( ch );
}

//-----------------------------------------------------
// Procedure:   ChangeDataView
//
//-----------------------------------------------------
void Tremolio9::ChangeDataView( int idata )
{
    char strVal[ 10 ] = {};

    if( idata == DATA_A )
        m_DataView = DATA_A;
    else if( idata == DATA_B )
        m_DataView = DATA_B;
    else
        return;

    snprintf( strVal, sizeof(strVal), "view %d", idata );
    m_pTextLabel->text = strVal;

    m_pEnvelope->m_EditData = GetViewEditData( m_DataView );

    ChangeChannel( m_CurrentChannel );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Tremolio9::process(const ProcessArgs &args)
{
    int ch;
    bool bTrig = false;
    float fval, famount, vibfreq;

    if( !m_bInitialized )
        return;

    m_BeatCount++;

    m_speedphase[ 0 ] += params[ PARAM_SPEED ].getValue() * nMAXSPEED;

    if(m_speedphase[0] >= APP->engine->getSampleRate())
    {
        m_speedphase[0] = m_speedphase[0] - APP->engine->getSampleRate();

        // track clock period
        m_EditDataFreq->setBeatLen( m_BeatCount );
        m_EditDataAmount->setBeatLen( m_BeatCount );
        m_BeatCount = 0;
    }

    // process each channel
    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        // trig, clock reset
        bTrig = ( m_SchTrigChTrig[ ch ].process( inputs[ INPUT_CH_TRIG + ch ].getNormalVoltage( 0.0f ) ) );

        if( bTrig )
            m_pButtonTrig[ ch ]->Set( true );

        if(bTrig || m_bTrig[ch])
        {
            m_phase[ ch ] = 0;
            bTrig = true;
        }

        // get frequency
        vibfreq = m_EditDataFreq->procStep( ch, bTrig, false );

        // get amount
        famount = m_EditDataAmount->procStep( ch, bTrig, false );

        // out wave
        fval = m_BufferWave[ int( ( m_phase[ ch ] * m_WaveRat ) + 0.5 ) ];

        m_phase[ ch ] += params[ PARAM_MAX_FREQ + ch ].getValue() * vibfreq * nMAXFREQ;

        if( m_phase[ ch ] >= APP->engine->getSampleRate() )
            m_phase[ ch ] = m_phase[ ch ] - APP->engine->getSampleRate();

        // process envelope
        outputs[ OUTPUT_CV + ch ].setVoltage( fval * famount * CV_MAX10);

        m_bTrig[ ch ] = false;
    }
}

Model *modelTremolio9 = createModel<Tremolio9, Tremolio9_Widget>( "Tremolio9" );
