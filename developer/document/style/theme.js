/*
  Global Theme Definition.
  Applies colors and variables to the document Root and Body.
*/
( function(){
  const RT = window.StyleRT = window.StyleRT || {};
  
  RT.theme = function(){
    RT.config = RT.config || {};
    
    // Define the Palette
    RT.config.theme = {
      background: "#1a1a1a"
      ,text: "#f0f0f0"
      ,accent: "#ffcc00"        
      ,code_bg: "#2d2d2d"
    };

    const palette = RT.config.theme;
    const html = document.documentElement;
    const body = document.body;

    if(RT.debug) RT.debug.log('style' ,'Applying Global Theme to Body/HTML.');

    // 1. Reset the "White Border" (Browser Defaults)
    // Browsers often add an 8px margin to body. We must zero this out.
    html.style.margin = "0";
    html.style.padding = "0";
    body.style.margin = "0";
    body.style.padding = "0";
    
    // Ensure the background covers the whole viewport, even if content is short
    body.style.minHeight = "100vh"; 

    // 2. Apply Palette
    // We paint both HTML and BODY to ensure over-scroll areas match.
    html.style.backgroundColor = palette.background;
    body.style.backgroundColor = palette.background;
    body.style.color = palette.text;
    
    // 3. Set Global CSS Variables
    // These will now be available anywhere in the document.
    body.style.setProperty("--rt-accent" ,palette.accent);
    body.style.setProperty("--rt-code-bg" ,palette.code_bg);
  };
} )();
