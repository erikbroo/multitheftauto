/*****************************************************************************
*
*  PROJECT:		Multi Theft Auto v1.0
*  LICENSE:		See LICENSE in the top level directory
*  FILE:		core/CChat.cpp
*  PURPOSE:		In-game chat box user interface implementation
*  DEVELOPERS:	Jax <>
*               arc_
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::vector;

extern CCore* g_pCore;

CChat * g_pChat = NULL;

CChat::CChat ( CGUI* pManager, CVector2D & vecPosition )
{    
    g_pChat = this;
    m_pManager = pManager;  

    // Calculate relative position (assuming a 800x600 native resolution for our defined CCHAT_* values)
    CVector2D vecResolution = m_pManager->GetResolution ();
    m_vecScale = CVector2D ( vecResolution.fX / 800.0f, vecResolution.fY / 600.0f );
    vecPosition = vecPosition * vecResolution;
    m_vecBackgroundPosition = vecPosition;

    // Initialize variables
    m_uiMostRecentLine = 0;
    m_uiScrollOffset = 0;
    m_TextColor = CHAT_TEXT_COLOR;
    m_bUseCEGUI = false;
    m_iCVarsRevision = -1;
    m_bVisible = false;
    m_bInputVisible = false;
    m_pFont = m_pManager->GetClearFont ();
    m_pDXFont = g_pCore->GetGraphics ()->GetFont ();
    m_fNativeWidth = CHAT_WIDTH;
    m_bCanChangeWidth = true;

    // Background area
    m_pBackground = m_pManager->CreateStaticImage ();
    m_pBackgroundTexture = m_pManager->CreateTexture ();
    m_pBackground->LoadFromTexture ( m_pBackgroundTexture );
    m_pBackground->MoveToBack ();
    m_pBackground->SetPosition ( m_vecBackgroundPosition );
    m_pBackground->SetSize ( m_vecBackgroundSize );
    m_pBackground->SetEnabled ( false );
    m_pBackground->SetVisible ( false );

    // Input area
    m_pInput = m_pManager->CreateStaticImage ();
    m_pInputTexture = m_pManager->CreateTexture ();
    m_pInput->LoadFromTexture ( m_pInputTexture );
    m_pInput->MoveToBack ();
    m_pInput->SetPosition ( m_vecInputPosition );
    m_pInput->SetSize ( m_vecInputSize );
    m_pInput->SetEnabled ( false );
    m_pInput->SetVisible ( false );
    SetInputPrefix ( "Say: " );

    // Set handlers
    m_pManager->SetCharacterKeyHandler ( GUI_CALLBACK_KEY ( &CChat::CharacterKeyHandler, this ) );

    // Load cvars and position the GUI
    LoadCVars ();
    UpdateGUI ();
}


CChat::~CChat ( void )
{
    Clear ();
    ClearInput ();

    SAFE_DELETE ( m_pBackground );
    SAFE_DELETE ( m_pBackgroundTexture );
    SAFE_DELETE ( m_pInput );
    SAFE_DELETE ( m_pInputTexture );

    if ( g_pChat == this )
        g_pChat = NULL;
}


void CChat::LoadCVars ( void )
{
    unsigned int Font;
    float fWidth = 1;

    CVARS_GET ( "chat_color",                   m_Color );              if( m_bCanChangeWidth ) SetColor ( m_Color );
    CVARS_GET ( "chat_input_color",             m_InputColor );         SetInputColor ( m_InputColor );
    CVARS_GET ( "chat_input_text_color",        m_InputTextColor );
    CVARS_GET ( "chat_use_cegui",               m_bUseCEGUI );
    CVARS_GET ( "chat_lines",                   m_uiNumLines );         SetNumLines ( m_uiNumLines);
    CVARS_GET ( "chat_text_color",              m_TextColor );
    CVARS_GET ( "chat_scale",                   m_vecScale );
    CVARS_GET ( "chat_width",                   fWidth );               if( m_bCanChangeWidth ) m_fNativeWidth = fWidth * CHAT_WIDTH;
    CVARS_GET ( "chat_css_style_text",          m_bCssStyleText );
    CVARS_GET ( "chat_css_style_background",    m_bCssStyleBackground );
    CVARS_GET ( "chat_line_life",               (unsigned int &)m_ulChatLineLife );
    CVARS_GET ( "chat_line_fade_out",           (unsigned int &)m_ulChatLineFadeOut );
    CVARS_GET ( "chat_font",                    (unsigned int &)Font ); SetChatFont ( (eChatFont)Font );
}


void CChat::Draw ( void )
{
    // Are we visible?
    if ( !m_bVisible )
        return;

    // Is it time to update all the chat related cvars?
    if( m_iCVarsRevision != CClientVariables::GetSingleton ().GetRevision () ) {
        m_iCVarsRevision = CClientVariables::GetSingleton ().GetRevision ();
        LoadCVars ();
        UpdateGUI ();
    }

    float fLineDifference = CChat::GetFontHeight ( m_vecScale.fY );
    CVector2D vecPosition ( m_vecBackgroundPosition.fX + ( 5.0f * m_vecScale.fX ), m_vecBackgroundPosition.fY + m_vecBackgroundSize.fY - ( fLineDifference * 1.25f ) );
    unsigned char ucAlpha = 0;
    unsigned long ulTime = GetTickCount ();
    unsigned long ulLineAge = 0;
    bool bShadow = ( m_Color.A == 0 || ( m_bCssStyleText && !m_bInputVisible ) );
    bool bInputShadow = ( m_InputColor.A == 0 );

    bool bDrawBackground = false;
    if ( m_bCssStyleBackground )
        bDrawBackground = ( m_bVisible && m_bInputVisible && m_Color.A != 0 );
    else
        bDrawBackground = ( m_bVisible && m_Color.A != 0 );

    if ( bDrawBackground )
    {
        // Hack to draw the background behind the text.
        m_pBackground->SetVisible ( true );
        m_pBackground->Render ();
        m_pBackground->SetVisible ( false );
    }

    if ( m_bInputVisible )
    {
        // ChrML: Hack so chatbox input always works. It might get unfocused..
        if ( !m_pBackground->IsActive () )
        {
            m_pBackground->Activate ();
        }
    }

    unsigned int uiLine = (m_uiMostRecentLine + m_uiScrollOffset) % CHAT_MAX_LINES;
    unsigned int uiLinesDrawn = 0;
    // Loop over the circular buffer
    while ( m_Lines [ uiLine ].IsActive () && uiLinesDrawn < m_uiNumLines )
    {
        ucAlpha = 255;
        if ( m_bCssStyleText && !m_bInputVisible )
        {
            ulLineAge = ulTime - m_Lines [ uiLine ].GetCreationTime ();
            if ( ulLineAge > m_ulChatLineLife )
            {
                if ( ulLineAge > ( m_ulChatLineLife + m_ulChatLineFadeOut ) )
                {
                    ucAlpha = 0;
                }
                else
                {
                    float fOver = float ( ( m_ulChatLineLife + m_ulChatLineFadeOut ) - ulLineAge );
                    fOver /= ( float ) m_ulChatLineFadeOut;
                    ucAlpha = unsigned char ( fOver * 255.0f );
                }
            }
        }

        if ( ucAlpha > 0 )
            m_Lines [ uiLine ].Draw ( vecPosition, ucAlpha, bShadow );

        vecPosition.fY -= fLineDifference;

        uiLine = (uiLine + 1) % CHAT_MAX_LINES;
        uiLinesDrawn++;
        if ( uiLine == m_uiMostRecentLine )     // Went through all lines?
            break;
    }

    if ( m_bInputVisible )
    {
        if ( m_InputColor.A != 0 )
        {
            if ( m_pInput )
            {
                // Hack to draw the input background behind the text.
                m_pInput->SetVisible ( true );
                m_pInput->Render ();
                m_pInput->SetVisible ( false );
            }
        }

        CVector2D vecPosition ( m_vecInputPosition.fX + ( 5.0f * m_vecScale.fX ), m_vecInputPosition.fY + ( fLineDifference * 0.125f ) );
        m_InputLine.Draw ( vecPosition, 255, bInputShadow );
    }
}


void CChat::Output ( const char* szText, bool bColorCoded )
{
    CChatLine* pLine = NULL;
    const char* szRemainingText = szText;
    CColor color = m_TextColor;
    do
    {
        m_uiMostRecentLine = (m_uiMostRecentLine == 0 ? CHAT_MAX_LINES - 1 : m_uiMostRecentLine - 1);
        pLine = &m_Lines [ m_uiMostRecentLine ];
        szRemainingText = pLine->Format ( szRemainingText,
            ( m_vecBackgroundSize.fX - ( 10.0f * m_vecScale.fX ) ), color, bColorCoded );
        pLine->SetActive ( true );
        pLine->UpdateCreationTime ();
    }
    while ( szRemainingText );
}


void CChat::Outputf ( bool bColorCoded, const char* szFormat, ... )
{
    SString str;
    
	va_list ap;
	va_start ( ap, szFormat );
    str.vFormat ( szFormat, ap );
	va_end ( ap );

    Output ( str.c_str (), bColorCoded );
}


void CChat::Clear ( void )
{
    for ( int i = 0; i < CHAT_MAX_LINES; i++ )
    {
        m_Lines [ i ].SetActive ( false );
    }
    m_uiMostRecentLine = 0;
}


void CChat::ClearInput ( void )
{
    m_strInputText.clear ();
    m_InputLine.Clear ();
    m_vecInputSize = CVector2D ( m_vecBackgroundSize.fX, ( GetFontHeight ( m_vecScale.fY ) * ( ( float ) m_InputLine.m_ExtraLines.size () + 1.25f ) ) );
    if ( m_pInput )
        m_pInput->SetSize ( m_vecInputSize );
}

// Not yet integrated/tested
void CChat::ScrollUp ()
{
    if ( m_Lines [ (m_uiMostRecentLine + m_uiScrollOffset + m_uiNumLines) % CHAT_MAX_LINES ].IsActive ()
        && !(
            ((m_uiMostRecentLine + m_uiScrollOffset) % CHAT_MAX_LINES < m_uiMostRecentLine) &&
            ((m_uiMostRecentLine + m_uiScrollOffset + m_uiNumLines) % CHAT_MAX_LINES >= m_uiMostRecentLine)
            )
        )
    {
        m_uiScrollOffset += m_uiNumLines;
    }
}

// Not yet integrated/tested
void CChat::ScrollDown ()
{
    if ( m_uiNumLines >= m_uiScrollOffset )
    {
        m_uiScrollOffset = 0;
    }
    else
    {
        m_uiScrollOffset -= m_uiNumLines;
    }
}

bool CChat::CharacterKeyHandler ( CGUIKeyEventArgs KeyboardArgs )
{
    // If we can take input
    if ( CLocalGUI::GetSingleton ().GetVisibleWindows () == 0 &&
        !CLocalGUI::GetSingleton ().GetConsole ()->IsVisible () &&
        m_bInputVisible )
    {
        // Check if it's a special key like enter and backspace, if not, add it as a character to the message
        switch ( KeyboardArgs.codepoint )
        {
            case VK_BACKSPACE:
            {
                if ( m_strInputText.size () > 0 )
                {
                    m_strInputText.resize ( m_strInputText.size () - 1 );
                    SetInputText ( m_strInputText.c_str () );
                }
                break;
            }

            case VK_RETURN:
            {
                // Empty the chat and hide the input stuff
                // If theres a command to call, call it
                if ( !m_strCommand.empty () && !m_strInputText.empty () )
                    CCommands::GetSingleton().Execute ( m_strCommand.c_str (), m_strInputText.c_str () );
    		
				// Deactivate the VisibleWindows counter
				CLocalGUI::GetSingleton ().SetVisibleWindows ( false );
                SetInputVisible ( false );

                break;
            }
            
            default:
            {
                // If we haven't exceeded the maximum number of characters per chat message, append the char to the message and update the input control
                if ( m_strInputText.size () < CHAT_MAX_CHAT_LENGTH )
                {                    
                    if ( KeyboardArgs.codepoint >= 32 && KeyboardArgs.codepoint <= 126 )
                    {
                        m_strInputText += static_cast < char > ( KeyboardArgs.codepoint );
                        SetInputText ( m_strInputText.c_str () );
                    }
                }
                break;
            }
        }
    }

	return true;
}


void CChat::SetVisible ( bool bVisible )
{
    m_bVisible = bVisible;
}


void CChat::SetInputVisible ( bool bVisible )
{    
    if ( !IsVisible () )
        bVisible = false;

    if ( bVisible )
    {
        ClearInput ();
    }

    m_bInputVisible = bVisible;
}


void CChat::SetNumLines ( unsigned int uiNumLines )
{
    if ( uiNumLines <= CHAT_MAX_LINES )
    {
        m_uiNumLines = uiNumLines;
        UpdateGUI ();
    }
}


void CChat::SetChatFont ( eChatFont Font )
{
    CGUIFont* pFont = g_pCore->GetGUI ()->GetDefaultFont ();
    ID3DXFont* pDXFont = g_pCore->GetGraphics ()->GetFont ();
    switch ( Font )
    {
        case CHAT_FONT_DEFAULT:
            pFont = g_pCore->GetGUI ()->GetDefaultFont ();
			pDXFont = g_pCore->GetGraphics ()->GetFont ( FONT_DEFAULT );
            break;
        case CHAT_FONT_CLEAR:
            pFont = g_pCore->GetGUI ()->GetClearFont ();
			pDXFont = g_pCore->GetGraphics ()->GetFont ( FONT_CLEAR );
            break;
        case CHAT_FONT_BOLD:
            pFont = g_pCore->GetGUI ()->GetBoldFont ();
            pDXFont = g_pCore->GetGraphics ()->GetFont ( FONT_DEFAULT_BOLD );
            break;
        case CHAT_FONT_ARIAL:
			pDXFont = g_pCore->GetGraphics ()->GetFont ( FONT_ARIAL );
            break;                
    }

    // Set fonts
    m_pFont = pFont;
    m_pDXFont = pDXFont;
}


void CChat::UpdateGUI ( void )
{
    m_vecBackgroundSize = CVector2D (
        m_fNativeWidth * m_vecScale.fX,
        CChat::GetFontHeight ( m_vecScale.fY ) * (float(m_uiNumLines) + 0.5f)
    );
    m_pBackground->SetSize ( m_vecBackgroundSize );

    m_vecInputPosition = CVector2D (
        m_vecBackgroundPosition.fX,
        m_vecBackgroundPosition.fY + m_vecBackgroundSize.fY
    );
    m_vecInputSize = CVector2D (
        m_vecBackgroundSize.fX,
        CChat::GetFontHeight ( m_vecScale.fY ) * 1.25f
    );
    if ( m_pInput )
    {
        m_pInput->SetPosition ( m_vecInputPosition );
        m_pInput->SetSize ( m_vecInputSize );
    }
}


void CChat::SetColor ( CColor& Color )
{
    unsigned long ulBackgroundColor = COLOR_ARGB ( Color.A, Color.R, Color.G, Color.B );

    m_pBackgroundTexture->LoadFromMemory ( &ulBackgroundColor, 1, 1 );
    m_pBackground->LoadFromTexture ( m_pBackgroundTexture );
}


void CChat::SetInputColor ( CColor& Color )
{
    unsigned long ulInputColor = COLOR_ARGB ( Color.A, Color.R, Color.G, Color.B );

    if ( m_pInputTexture )
        m_pInputTexture->LoadFromMemory ( &ulInputColor, 1, 1 );

    if ( m_pInput )
        m_pInput->LoadFromTexture ( m_pInputTexture );
}


const char* CChat::GetInputPrefix ( void )
{
    return m_InputLine.m_Prefix.GetText ();
}


void CChat::SetInputPrefix ( const char* szPrefix )
{
    m_InputLine.m_Prefix.SetText ( szPrefix );
}

void CChat::SetInputText ( const char* szText )
{
    m_InputLine.Clear ();
    
    CColor color = m_InputTextColor;
    const char* szRemainingText = m_InputLine.Format ( szText,
        ( m_vecInputSize.fX - ( 10.0f * m_vecScale.fX ) - m_InputLine.m_Prefix.GetWidth () ),
        color, false );

    CChatLine* pLine = NULL;
    
    while ( szRemainingText && m_InputLine.m_ExtraLines.size () < 3 )
    {
        m_InputLine.m_ExtraLines.resize ( m_InputLine.m_ExtraLines.size () + 1 );
        CChatLine& line = *(m_InputLine.m_ExtraLines.end () - 1);
        szRemainingText = line.Format ( szRemainingText,
            ( m_vecInputSize.fX - ( 10.0f * m_vecScale.fX ) ), color, false );
    }

    if ( szText != m_strInputText.c_str () )
        m_strInputText = szText;

    if ( szRemainingText )
        m_strInputText.resize ( szRemainingText - szText );

    m_vecInputSize = CVector2D ( m_vecBackgroundSize.fX, ( CChat::GetFontHeight ( m_vecScale.fY ) * ( ( float ) m_InputLine.m_ExtraLines.size () + 1.25f ) ) );
    if ( m_pInput )
        m_pInput->SetSize ( m_vecInputSize );
}


void CChat::SetCommand ( const char* szCommand )
{
    if ( !szCommand )
        return;

    m_strCommand = szCommand;

    if ( strcmp ( szCommand, "chatboxsay" ) == 0 )
    {
        SetInputPrefix ( "Say: " );
    }
    else
    {
        std::string strPrefix = m_strCommand + ": ";
        strPrefix[0] = toupper ( strPrefix[0] );
        SetInputPrefix ( strPrefix.c_str () );
    }
}


float CChat::GetFontHeight ( float fScale )
{
    if ( !g_pChat )
        return 0.0f;

    if ( g_pChat->m_bUseCEGUI )
    {
        return g_pChat->m_pFont->GetFontHeight ( fScale );
    }
    return g_pCore->GetGraphics ()->GetDXFontHeight ( fScale, g_pChat->m_pDXFont );
}


float CChat::GetCharacterWidth ( int iChar, float fScale )
{
    if ( !g_pChat )
        return 0.0f;

    if ( g_pChat->m_bUseCEGUI )
    {
        return g_pChat->m_pFont->GetCharacterWidth ( iChar, fScale );
    }
    return g_pCore->GetGraphics ()->GetDXCharacterWidth ( ( char ) iChar, fScale, g_pChat->m_pDXFont );
}


float CChat::GetTextExtent ( const char * szText, float fScale )
{
    if ( !g_pChat )
        return 0.0f;

    if ( g_pChat->m_bUseCEGUI )
    {
        return g_pChat->m_pFont->GetTextExtent ( szText, fScale );
    }
    return g_pCore->GetGraphics ()->GetDXTextExtent ( szText, fScale, g_pChat->m_pDXFont );
}


void CChat::DrawTextString ( const char * szText, CRect2D DrawArea, float fZ, CRect2D ClipRect, unsigned long ulFormat, unsigned long ulColor, float fScaleX, float fScaleY )
{
    if ( !g_pChat )
        return;

    if ( g_pChat->m_bUseCEGUI )
    {
        g_pChat->m_pFont->DrawTextString ( szText, DrawArea, fZ, ClipRect, ulFormat, ulColor, fScaleX, fScaleY );
    }
    else
    {
        g_pCore->GetGraphics ()->DrawText ( ( int ) DrawArea.fX1, ( int ) DrawArea.fY1, ( int ) DrawArea.fX1, ( int ) DrawArea.fY1, ulColor, szText, fScaleX, fScaleY, DT_LEFT | DT_TOP | DT_NOCLIP, g_pChat->m_pDXFont );
    }
}


CChatLine::CChatLine ( void )
{
    m_bActive = false;
    UpdateCreationTime ();
}

void CChatLine::UpdateCreationTime ()
{
    m_ulCreationTime = GetTickCount ();
}

bool CChatLine::IsColorCode ( const char* szColorCode )
{
    if ( *szColorCode != '#' )
        return false;

    bool bValid = true;
    for ( int i = 0; i < 6; i++ )
    {
        char c = szColorCode [ 1 + i ];
        if ( !isdigit ( c ) && (c < 'A' || c > 'F') && (c < 'a' || c > 'f') )
        {
            bValid = false;
            break;
        }
    }
    return bValid;
}


const char* CChatLine::Format ( const char* szString, float fWidth, CColor& color, bool bColorCoded )
{
    float fCurrentWidth = 0.0f;
    m_Sections.clear ();

    const char* szSectionStart = szString;
    const char* szSectionEnd = szString;
    const char* szLastWrapPoint = szString;
    bool bLastSection = false;
    while ( !bLastSection )      // iterate over sections
    {
        m_Sections.resize ( m_Sections.size () + 1 );
        CChatLineSection& section = *(m_Sections.end () - 1);
        section.SetColor ( color );

        if ( m_Sections.size () > 1 && bColorCoded)      // If we've processed sections before
            szSectionEnd += 7;                           // skip the color code

        szSectionStart = szSectionEnd;
        szLastWrapPoint = szSectionStart;

        while ( true )      // find end of this section
        {
            float fCharWidth = CChat::GetCharacterWidth ( *szSectionEnd, g_pChat->m_vecScale.fX );
            if ( *szSectionEnd == '\0' || *szSectionEnd == '\n' || fCurrentWidth + fCharWidth > fWidth )
            {
                bLastSection = true;
                break;
            }
            if ( bColorCoded && IsColorCode ( szSectionEnd ) )
            {
                unsigned long ulColor = 0;
                sscanf ( szSectionEnd + 1, "%06x", &ulColor );
                color = ulColor;
                break;
            }
            if ( isspace ( *szSectionEnd ) || ispunct ( *szSectionEnd ) )
            {
                szLastWrapPoint = szSectionEnd;
            }
            fCurrentWidth += fCharWidth;
            szSectionEnd++;
        }
        section.m_strText.assign ( szSectionStart, szSectionEnd - szSectionStart );
    }

    if ( *szSectionEnd == '\0' )
    {
        return NULL;
    }
    else if( *szSectionEnd == '\n' )
    {
        return szSectionEnd + 1;
    }
    else
    {
        // Do word wrap
        if ( szLastWrapPoint == szSectionStart )
        {
            // Wrapping point coincides with the start of a section.
            if ( szLastWrapPoint == szString )
            {
                // The line consists of one huge word. Leave the one section we created as it
                // is (with the huge word cut off) and return szRemaining as the rest of the word
                return szSectionEnd;
            }
            else
            {
                // There's more than one section, remove the last one (where our wrap point is)
                m_Sections.pop_back ();
            }
        }
        else
        {
            // Wrapping point is in the middle of a section, truncate
            (*(m_Sections.end () - 1)).m_strText.resize ( szLastWrapPoint - szSectionStart );
        }
        return szLastWrapPoint;
    }
}


void CChatLine::Draw ( CVector2D& vecPosition, unsigned char ucAlpha, bool bShadow )
{
    float fCurrentX = vecPosition.fX;
    std::vector < CChatLineSection >::iterator iter = m_Sections.begin ();
    for ( ; iter != m_Sections.end () ; iter++ )
    {
        (*iter).Draw ( CVector2D ( fCurrentX, vecPosition.fY ), ucAlpha, bShadow );
        fCurrentX += (*iter).GetWidth ();
    }
}


float CChatLine::GetWidth ()
{
    float fWidth = 0.0f;
    std::vector < CChatLineSection >::iterator it;
    for ( it = m_Sections.begin (); it != m_Sections.end (); it++ )
    {
        fWidth += (*it).GetWidth ();
    }
    return fWidth;
}

void CChatLine::RemoveColorCode ( const char* szString, std::string& strOut )
{
    strOut.clear ();
    const char* szStart = szString;
    const char* szEnd = szString;

    while ( true )
    {
        if ( *szEnd == '\0' )
        {
            strOut.append ( szStart, szEnd - szStart );
            break;
        }
        else if ( IsColorCode ( szEnd ) )
        {
            strOut.append ( szStart, szEnd - szStart );
            szStart = szEnd + 7;
            szEnd = szStart;
        }
        else
        {
            szEnd++;
        }
    }
}

void CChatInputLine::Draw ( CVector2D& vecPosition, unsigned char ucAlpha, bool bShadow )
{
    CColor colPrefix;
    m_Prefix.GetColor ( colPrefix );
    if ( colPrefix.A > 0 )
        m_Prefix.Draw ( vecPosition, colPrefix.A, bShadow );
    
    if ( g_pChat->m_InputColor.A > 0 && m_Sections.size () > 0 )
    {
        m_Sections [ 0 ].Draw ( CVector2D ( vecPosition.fX + m_Prefix.GetWidth (), vecPosition.fY ),
            g_pChat->m_InputTextColor.A, bShadow );

        float fLineDifference = CChat::GetFontHeight ( g_pChat->m_vecScale.fY );

        vector < CChatLine >::iterator iter = m_ExtraLines.begin ();
        for ( ; iter != m_ExtraLines.end () ; iter++ )
        {
            vecPosition.fY += fLineDifference;
            (*iter).Draw ( vecPosition, g_pChat->m_InputColor.A, bShadow );        
        }
    }
}


void CChatInputLine::Clear ( void )
{
    m_Sections.clear ();
    m_ExtraLines.clear ();
}

CChatLineSection::CChatLineSection ()
{
    m_fCachedWidth = -1.0f;
    m_uiCachedLength = 0;
}

CChatLineSection::CChatLineSection ( const CChatLineSection& other )
{
    *this = other;
}

CChatLineSection& CChatLineSection::operator = ( const CChatLineSection& other )
{
    m_strText = other.m_strText;
    m_Color = other.m_Color;
    m_fCachedWidth = other.m_fCachedWidth;
    m_uiCachedLength = other.m_uiCachedLength;
    return *this;
}

void CChatLineSection::Draw ( CVector2D& vecPosition, unsigned char ucAlpha, bool bShadow )
{
    if ( !m_strText.empty () && ucAlpha > 0 )
    {
        if ( bShadow )
        {
            CRect2D drawShadowAt ( vecPosition.fX + 1.0f, vecPosition.fY + 1.0f, vecPosition.fX + 1000.0f, vecPosition.fY + 1000.0f );
            CChat::DrawTextString ( m_strText.c_str (), drawShadowAt, 0.0f, drawShadowAt, 0, COLOR_ARGB ( ucAlpha, 0, 0, 0 ), g_pChat->m_vecScale.fX, g_pChat->m_vecScale.fY );                
        }
        CRect2D drawAt ( vecPosition.fX, vecPosition.fY, vecPosition.fX + 1000.0f, vecPosition.fY + 1000.0f );
        CChat::DrawTextString ( m_strText.c_str (), drawAt, 0.0f, drawAt, 0, COLOR_ARGB ( ucAlpha, m_Color.R, m_Color.G, m_Color.B ), g_pChat->m_vecScale.fX, g_pChat->m_vecScale.fY );
    }
}


float CChatLineSection::GetWidth ()
{
    if ( m_fCachedWidth < 0.0f || m_strText.size () != m_uiCachedLength )
    {
        m_fCachedWidth = 0.0f;
        for ( unsigned int i = 0; i < m_strText.size (); i++ )
        {
            m_fCachedWidth += CChat::GetCharacterWidth ( m_strText [ i ], g_pChat->m_vecScale.fX );            
        }
        m_uiCachedLength = m_strText.size ();
    }
    return m_fCachedWidth;
}