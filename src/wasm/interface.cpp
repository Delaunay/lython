#ifndef BUILD_WEBASSEMBLY
#define BUILD_WEBASSEMBLY 1
#endif

#if BUILD_WEBASSEMBLY
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "parser/parser.h"
#include "sema/sema.h"
#include "vm/tree.h"

namespace js {

using AnimId      = emscripten::val;
using TimerHandle = emscripten::val;
using str         = std::string;

//! The window object represents an open window in a browser.
struct Window {

    /*
    // Properties
    closed 	        Returns a boolean true if a window is closed.
    console 	    Returns the Console Object for the window.
                    See also The Console Object.
    defaultStatus 	Deprecated.
    document 	    Returns the Document object for the window.
                    See also The Document Object.
    frameElement 	Returns the frame in which the window runs.
    frames 	        Returns all window objects running in the window.
    history 	    Returns the History object for the window.
                    See also The History Object.
    innerHeight 	Returns the height of the window's content area (viewport) including
    scrollbars innerWidth 	Returns the width of a window's content area (viewport) including
    scrollbars length 	        Returns the number of <iframe> elements in the current window
    localStorage 	Allows to save key/value pairs in a web browser. Stores the data with no
    expiration date location 	    Returns the Location object for the window. See also the The
    Location Object. name 	        Sets or returns the name of a window navigator Returns the
    Navigator object for the window. See also The Navigator object. opener 	        Returns a
    reference to the window that created the window outerHeight 	Returns the height of the
    browser window, including toolbars/scrollbars outerWidth 	    Returns the width of the browser
    window, including toolbars/scrollbars pageXOffset 	Returns the pixels the current document has
    been scrolled (horizontally) from the upper left corner of the window pageYOffset 	Returns the
    pixels the current document has been scrolled (vertically) from the upper left corner of the
    window parent 	        Returns the parent window of the current window screen Returns the
    Screen object for the window See also The Screen object screenLeft 	    Returns the horizontal
    coordinate of the window relative to the screen screenTop 	    Returns the vertical coordinate
    of the window relative to the screen screenX 	    Returns the horizontal coordinate of the
    window relative to the screen screenY 	    Returns the vertical coordinate of the window
    relative to the screen sessionStorage 	Allows to save key/value pairs in a web browser.
    Stores the data for one session scrollX 	    An alias of pageXOffset scrollY 	    An alias
    of pageYOffset self 	        Returns the current window status 	        Deprecated.
    Avoid using it. top 	        Returns the topmost browser window
    */

    Window(emscripten::val window = emscripten::val::global("window"));

    emscripten::val window;

    //! Attaches an event handler to the window
    void addEventListener(str const& event, std::function<void()> callback, bool capture = false);

    //!  Removes an event handler from the window
    void removeEventListener(str const& event, std::function<void()> callback, bool capture = false);

    //!  Displays an alert box with a message and an OK button
    void alert(str const& msg);

    //!  Displays a dialog box with a message and an OK and a Cancel button
    bool confirm(str const& msg);
    //! Displays a dialog box that prompts the visitor for input
    str prompt(str const& msg, str const& default_text);

    //! Decodes a base-64 encoded string
    str atob(str const& base64);

    //! Encodes a string in base-64
    str btoa(str const& data);

    //! Returns a MediaQueryList object representing the specified CSS media query string
    emscripten::val matchMedia(str const& media);

    //! Moves a window relative to its current position
    void moveBy(float x, float y);

    //! Moves a window to the specified position
    void moveTo(float x, float y);

    //! Opens a new browser window
    std::optional<Window> open(str const& url, str const& name, str const& specs);

    //! Removes focus from the current window
    void blur();

    //! Sets focus to the current window
    void focus();

    //! Closes the current window
    void close();

    //! Prints the content of the current window
    void print();

    //! Requests the browser to call a function to update an animation before the next repaint
    AnimId requestAnimationFrame(std::function<void(float)> callback);

    void cancelAnimationFrame(AnimId id);

    //!  Resizes the window by the specified pixels
    void resizeBy(float x, float y);

    //! Resizes the window to the specified width and height
    void resizeTo(float x, float y);

    //! Scrolls the document by the specified number of pixels
    void scrollBy(float x, float y);

    //! Scrolls the document to the specified coordinates
    void scrollTo(float x, float y);

    //! alls a function or evaluates an expression at specified intervals (in milliseconds)
    template <typename Fun, typename... Args>
    TimerHandle setInterval(Fun callback, float milliseconds, Args... args);

    //! Clears a timer set with setInterval()
    void clearInterval(TimerHandle timer);

    //! Calls a function or evaluates an expression after a specified number of milliseconds
    template <typename... Args>
    TimerHandle setTimeout(void (*callback)(Args...), float milliseconds, Args... args);

    //!  Clears a timer set with setTimeout()
    void clearTimeout(TimerHandle timer);

    //! Stops the window from loading
    void stop();

    //! Gets the current computed CSS styles applied to an element
    // val getComputedStyle(element, pseudoElement)

    //! Deprecated. Use scrollTo() instead.
    // scroll()

    //! Returns a Selection object representing the range of text selected by the user
    // getSelection()

    /*
    //! The console object provides access to the browser's debugging console.
    Console* console();
    History* history();
    Location* location();
    Navigator* navigator();
    Screen* screen();
    */
};

struct Attribute {
    // Returns an attribute's name
    str name; 	
    
    // Sets or returns an attribute's value
    val value; 	
    
    // Returns true if the attribute is specified
    bool specified;
};

struct NamedNodeMap {
    //Returns an attribute node (by name) from a NamedNodeMap
    Attribute getNamedItem(str const& name);

    // Returns an attribute node (by index) from a NamedNodeMap
    Attribute item(int index); 	

    // Returns the number of attributes in a NamedNodeMap
    int length();

    // Removes an attribute (node)
    Attribute removeNamedItem(str const& nodename); 	

    // Sets an attribute (node) by name
    Attribute setNamedItem(Attribute node);
};

struct Element {

// Sets or returns the accesskey attribute of an element
#if 0
accessKey 	
addEventListener() 	Attaches an event handler to an element
appendChild() 	Adds (appends) a new child node to an element
attributes 	Returns a NamedNodeMap of an element's attributes
blur() 	Removes focus from an element
childElementCount 	Returns an elements's number of child elements
childNodes 	Returns a NodeList of an element's child nodes
children 	Returns an HTMLCollection of an element's child elements
classList 	Returns the class name(s) of an element
className 	Sets or returns the value of the class attribute of an element
click() 	Simulates a mouse-click on an element
clientHeight 	Returns the height of an element, including padding
clientLeft 	Returns the width of the left border of an element
clientTop 	Returns the width of the top border of an element
clientWidth 	Returns the width of an element, including padding
cloneNode() 	Clones an element
closest() 	Searches the DOM tree for the closest element that matches a CSS selector
compareDocumentPosition() 	Compares the document position of two elements
contains() 	Returns true if a node is a descendant of a node
contentEditable 	Sets or returns whether the content of an element is editable or not
dir 	Sets or returns the value of the dir attribute of an element
firstChild 	Returns the first child node of an element
firstElementChild 	Returns the first child element of an element
focus() 	Gives focus to an element
getAttribute() 	Returns the value of an element's attribute
getAttributeNode() 	Returns an attribute node
getBoundingClientRect() 	Returns the size of an element and its position relative to the viewport
getElementsByClassName() 	Returns a collection of child elements with a given class name
getElementsByTagName() 	Returns a collection of child elements with a given tag name
hasAttribute() 	Returns true if an element has a given attribute
hasAttributes() 	Returns true if an element has any attributes
hasChildNodes() 	Returns true if an element has any child nodes
id 	Sets or returns the value of the id attribute of an element
innerHTML 	Sets or returns the content of an element
innerText 	Sets or returns the text content of a node and its descendants
insertAdjacentElement() 	Inserts a new HTML element at a position relative to an element
insertAdjacentHTML() 	Inserts an HTML formatted text at a position relative to an element
insertAdjacentText() 	Inserts text into a position relative to an element
insertBefore() 	Inserts a new child node before an existing child node
isContentEditable 	Returns true if an element's content is editable
isDefaultNamespace() 	Returns true if a given namespaceURI is the default
isEqualNode() 	Checks if two elements are equal
isSameNode() 	Checks if two elements are the same node
isSupported() 	Deprecated
lang 	Sets or returns the value of the lang attribute of an element
lastChild 	Returns the last child node of an element
lastElementChild 	Returns the last child element of an element
matches() 	Returns true if an element is matched by a given CSS selector
namespaceURI 	Returns the namespace URI of an element
nextSibling 	Returns the next node at the same node tree level
nextElementSibling 	Returns the next element at the same node tree level
nodeName 	Returns the name of a node
nodeType 	Returns the node type of a node
nodeValue 	Sets or returns the value of a node
normalize() 	Joins adjacent text nodes and removes empty text nodes in an element
offsetHeight 	Returns the height of an element, including padding, border and scrollbar
offsetWidth 	Returns the width of an element, including padding, border and scrollbar
offsetLeft 	Returns the horizontal offset position of an element
offsetParent 	Returns the offset container of an element
offsetTop 	Returns the vertical offset position of an element
outerHTML 	Sets or returns the content of an element (including the start tag and the end tag)
outerText 	Sets or returns the outer text content of a node and its descendants
ownerDocument 	Returns the root element (document object) for an element
parentNode 	Returns the parent node of an element
parentElement 	Returns the parent element node of an element
previousSibling 	Returns the previous node at the same node tree level
previousElementSibling 	Returns the previous element at the same node tree level
querySelector() 	Returns the first child element that matches a CSS selector(s)
querySelectorAll() 	Returns all child elements that matches a CSS selector(s)
remove() 	Removes an element from the DOM
removeAttribute() 	Removes an attribute from an element
removeAttributeNode() 	Removes an attribute node, and returns the removed node
removeChild() 	Removes a child node from an element
removeEventListener() 	Removes an event handler that has been attached with the addEventListener() method
replaceChild() 	Replaces a child node in an element
scrollHeight 	Returns the entire height of an element, including padding
scrollIntoView() 	Scrolls the an element into the visible area of the browser window
scrollLeft 	Sets or returns the number of pixels an element's content is scrolled horizontally
// Sets or returns the number of pixels an element's content is scrolled vertically
scrollTop 	
// Returns the entire width of an element, including padding
scrollWidth 	
// Sets or changes an attribute's value
setAttribute() 	
//	Sets or changes an attribute node
setAttributeNode() 
// Sets or returns the value of the style attribute of an element
style 	
// Sets or returns the value of the tabindex attribute of an element
tabIndex 	
// Returns the tag name of an element
tagName 	
// Sets or returns the textual content of a node and its descendants
textContent 	
// Sets or returns the value of the title attribute of an element
title 	

// Converts an element to a string
toString() 	
#endif
};


struct Collection {
    // Returns the number of elements in an HTMLCollection
    int length();

    // Returns the element at a specified index
    Element item(int idx);

    // Returns the element with a specified id  
    Element namedItem(str const& id);

};

template<typename T, typename V>
struct Pair {};

template<typename T>
struct Iterator {};

struct NodeList {
    // Returns an Iterator with the key/value pairs from the list
    Iterator<Pair<str, Element>> entries() 	;

    // Executes a callback function for each node in the list
    void forEach(std::function<void()> fun);

    // Returns the node at a specified index
    Element item(int idx) 	;

    // 	Returns an Iterator with the keys from the list
    Iterator<str> keys() ;

    // Returns the number of nodes in a NodeList
    int length();

    // Returns an Iterator with the values from the list
    Iterator<Element> values();
};

struct TokenList {
    //! Adds one or more tokens to the list
    template<typename...Args>
    void add(Args... tokens); 	

    // Returns true if the list contains a class
    bool contains(str const& token); 	
    
    // Returns an Iterator with key/value pairs from the list
    Iterator<Pair<str, str>> entries();

    // Executes a callback function for each token in the list
    void forEach(std::function<void(str, int, TokenList)> fun, val value);

    // Returns the token at a specified index
    str item(int i);

    // Returns an Iterator with the keys in the list
    Iterator<str> keys();

    // Returns the number of tokens in the list
    int length(); 	

    // Removes one or more tokens from the list
    template<typename...Args>
    void remove(Args... tokens);

    // Replaces a token in the list
    bool replace(str const& old, str const& new_);

    // Returns true if a token is one of an attribute's supported tokens
    bool supports(str const& token);

    // Toggles between tokens in the list
    void toggle(str const& tok);

    // Returns the token list as a string
    str value(); 	
    
    // Returns an Iterator with the values in the list
    Iterator<str> values();
};

struct Style {
    #if 0
alignContent 	Sets or returns the alignment between the lines inside a flexible container when the items do not use all available space
alignItems 	Sets or returns the alignment for items inside a flexible container
alignSelf 	Sets or returns the alignment for selected items inside a flexible container
animation 	A shorthand property for all the animation properties below, except the animationPlayState property
animationDelay 	Sets or returns when the animation will start
animationDirection 	Sets or returns whether or not the animation should play in reverse on alternate cycles
animationDuration 	Sets or returns how many seconds or milliseconds an animation takes to complete one cycle
animationFillMode 	Sets or returns what values are applied by the animation outside the time it is executing
animationIterationCount 	Sets or returns the number of times an animation should be played
animationName 	Sets or returns a name for the @keyframes animation
animationTimingFunction 	Sets or returns the speed curve of the animation
animationPlayState 	Sets or returns whether the animation is running or paused
background 	Sets or returns all the background properties in one declaration
backgroundAttachment 	Sets or returns whether a background-image is fixed or scrolls with the page
backgroundColor 	Sets or returns the background-color of an element
backgroundImage 	Sets or returns the background-image for an element
backgroundPosition 	Sets or returns the starting position of a background-image
backgroundRepeat 	Sets or returns how to repeat (tile) a background-image
backgroundClip 	Sets or returns the painting area of the background
backgroundOrigin 	Sets or returns the positioning area of the background images
backgroundSize 	Sets or returns the size of the background image
backfaceVisibility 	Sets or returns whether or not an element should be visible when not facing the screen
border 	Sets or returns borderWidth, borderStyle, and borderColor in one declaration
borderBottom 	Sets or returns all the borderBottom properties in one declaration
borderBottomColor 	Sets or returns the color of the bottom border
borderBottomLeftRadius 	Sets or returns the shape of the border of the bottom-left corner
borderBottomRightRadius 	Sets or returns the shape of the border of the bottom-right corner
borderBottomStyle 	Sets or returns the style of the bottom border
borderBottomWidth 	Sets or returns the width of the bottom border
borderCollapse 	Sets or returns whether the table border should be collapsed into a single border, or not
borderColor 	Sets or returns the color of an element's border (can have up to four values)
borderImage 	A shorthand property for setting or returning all the borderImage properties
borderImageOutset 	Sets or returns the amount by which the border image area extends beyond the border box
borderImageRepeat 	Sets or returns whether the image-border should be repeated, rounded or stretched
borderImageSlice 	Sets or returns the inward offsets of the image-border
borderImageSource 	Sets or returns the image to be used as a border
borderImageWidth 	Sets or returns the widths of the image-border
borderLeft 	Sets or returns all the borderLeft properties in one declaration
borderLeftColor 	Sets or returns the color of the left border
borderLeftStyle 	Sets or returns the style of the left border
borderLeftWidth 	Sets or returns the width of the left border
borderRadius 	A shorthand property for setting or returning all the four borderRadius properties
borderRight 	Sets or returns all the borderRight properties in one declaration
borderRightColor 	Sets or returns the color of the right border
borderRightStyle 	Sets or returns the style of the right border
borderRightWidth 	Sets or returns the width of the right border
borderSpacing 	Sets or returns the space between cells in a table
borderStyle 	Sets or returns the style of an element's border (can have up to four values)
borderTop 	Sets or returns all the borderTop properties in one declaration
borderTopColor 	Sets or returns the color of the top border
borderTopLeftRadius 	Sets or returns the shape of the border of the top-left corner
borderTopRightRadius 	Sets or returns the shape of the border of the top-right corner
borderTopStyle 	Sets or returns the style of the top border
borderTopWidth 	Sets or returns the width of the top border
borderWidth 	Sets or returns the width of an element's border (can have up to four values)
bottom 	Sets or returns the bottom position of a positioned element
boxDecorationBreak 	Sets or returns the behaviour of the background and border of an element at page-break, or, for in-line elements, at line-break.
boxShadow 	Attaches one or more drop-shadows to the box
boxSizing 	Allows you to define certain elements to fit an area in a certain way
captionSide 	Sets or returns the position of the table caption
caretColor 	Sets or returns the caret/cursor color of an element
clear 	Sets or returns the position of the element relative to floating objects
clip 	Sets or returns which part of a positioned element is visible
color 	Sets or returns the color of the text
columnCount 	Sets or returns the number of columns an element should be divided into
columnFill 	Sets or returns how to fill columns
columnGap 	Sets or returns the gap between the columns
columnRule 	A shorthand property for setting or returning all the columnRule properties
columnRuleColor 	Sets or returns the color of the rule between columns
columnRuleStyle 	Sets or returns the style of the rule between columns
columnRuleWidth 	Sets or returns the width of the rule between columns
columns 	A shorthand property for setting or returning columnWidth and columnCount
columnSpan 	Sets or returns how many columns an element should span across
columnWidth 	Sets or returns the width of the columns
content 	Used with the :before and :after pseudo-elements, to insert generated content
counterIncrement 	Increments one or more counters
counterReset 	Creates or resets one or more counters
cursor 	Sets or returns the type of cursor to display for the mouse pointer
direction 	Sets or returns the text direction
display 	Sets or returns an element's display type
emptyCells 	Sets or returns whether to show the border and background of empty cells, or not
filter 	Sets or returns image filters (visual effects, like blur and saturation)
flex 	Sets or returns the length of the item, relative to the rest
flexBasis 	Sets or returns the initial length of a flexible item
flexDirection 	Sets or returns the direction of the flexible items
flexFlow 	A shorthand property for the flexDirection and the flexWrap properties
flexGrow 	Sets or returns how much the item will grow relative to the rest
flexShrink 	Sets or returns how the item will shrink relative to the rest
flexWrap 	Sets or returns whether the flexible items should wrap or not
cssFloat 	Sets or returns the horizontal alignment of an element
font 	Sets or returns fontStyle, fontVariant, fontWeight, fontSize, lineHeight, and fontFamily in one declaration
fontFamily 	Sets or returns the font family for text
fontSize 	Sets or returns the font size of the text
fontStyle 	Sets or returns whether the style of the font is normal, italic or oblique
fontVariant 	Sets or returns whether the font should be displayed in small capital letters
fontWeight 	Sets or returns the boldness of the font
fontSizeAdjust 	Preserves the readability of text when font fallback occurs
fontStretch 	Selects a normal, condensed, or expanded face from a font family
hangingPunctuation 	Specifies whether a punctuation character may be placed outside the line box
height 	Sets or returns the height of an element
hyphens 	Sets how to split words to improve the layout of paragraphs
icon 	Provides the author the ability to style an element with an iconic equivalent
imageOrientation 	Specifies a rotation in the right or clockwise direction that a user agent applies to an image
isolation 	Defines whether an element must create a new stacking content
justifyContent 	Sets or returns the alignment between the items inside a flexible container when the items do not use all available space.
left 	Sets or returns the left position of a positioned element
letterSpacing 	Sets or returns the space between characters in a text
lineHeight 	Sets or returns the distance between lines in a text
listStyle 	Sets or returns listStyleImage, listStylePosition, and listStyleType in one declaration
listStyleImage 	Sets or returns an image as the list-item marker
listStylePosition 	Sets or returns the position of the list-item marker
listStyleType 	Sets or returns the list-item marker type
margin 	Sets or returns the margins of an element (can have up to four values)
marginBottom 	Sets or returns the bottom margin of an element
marginLeft 	Sets or returns the left margin of an element
marginRight 	Sets or returns the right margin of an element
marginTop 	Sets or returns the top margin of an element
maxHeight 	Sets or returns the maximum height of an element
maxWidth 	Sets or returns the maximum width of an element
minHeight 	Sets or returns the minimum height of an element
minWidth 	Sets or returns the minimum width of an element
navDown 	Sets or returns where to navigate when using the arrow-down navigation key
navIndex 	Sets or returns the tabbing order for an element
navLeft 	Sets or returns where to navigate when using the arrow-left navigation key
navRight 	Sets or returns where to navigate when using the arrow-right navigation key
navUp 	Sets or returns where to navigate when using the arrow-up navigation key
objectFit 	Specifies how the contents of a replaced element should be fitted to the box established by its used height and width
objectPosition 	Specifies the alignment of the replaced element inside its box
opacity 	Sets or returns the opacity level for an element
order 	Sets or returns the order of the flexible item, relative to the rest
orphans 	Sets or returns the minimum number of lines for an element that must be left at the bottom of a page when a page break occurs inside an element
outline 	Sets or returns all the outline properties in one declaration
outlineColor 	Sets or returns the color of the outline around a element
outlineOffset 	Offsets an outline, and draws it beyond the border edge
outlineStyle 	Sets or returns the style of the outline around an element
outlineWidth 	Sets or returns the width of the outline around an element
overflow 	Sets or returns what to do with content that renders outside the element box
overflowX 	Specifies what to do with the left/right edges of the content, if it overflows the element's content area
overflowY 	Specifies what to do with the top/bottom edges of the content, if it overflows the element's content area
padding 	Sets or returns the padding of an element (can have up to four values)
paddingBottom 	Sets or returns the bottom padding of an element
paddingLeft 	Sets or returns the left padding of an element
paddingRight 	Sets or returns the right padding of an element
paddingTop 	Sets or returns the top padding of an element
pageBreakAfter 	Sets or returns the page-break behavior after an element
pageBreakBefore 	Sets or returns the page-break behavior before an element
pageBreakInside 	Sets or returns the page-break behavior inside an element
perspective 	Sets or returns the perspective on how 3D elements are viewed
perspectiveOrigin 	Sets or returns the bottom position of 3D elements
position 	Sets or returns the type of positioning method used for an element (static, relative, absolute or fixed)
quotes 	Sets or returns the type of quotation marks for embedded quotations
resize 	Sets or returns whether or not an element is resizable by the user
right 	Sets or returns the right position of a positioned element
scrollBehavior 	Specifies whether to smoothly animate the scroll position, instead of a straight jump, when the user clicks on a link within a scrollable boxt
tableLayout 	Sets or returns the way to lay out table cells, rows, and columns
tabSize 	Sets or returns the length of the tab-character
textAlign 	Sets or returns the horizontal alignment of text
textAlignLast 	Sets or returns how the last line of a block or a line right before a forced line break is aligned when text-align is "justify"
textDecoration 	Sets or returns the decoration of a text
textDecorationColor 	Sets or returns the color of the text-decoration
textDecorationLine 	Sets or returns the type of line in a text-decoration
textDecorationStyle 	Sets or returns the style of the line in a text decoration
textIndent 	Sets or returns the indentation of the first line of text
textJustify 	Sets or returns the justification method used when text-align is "justify"
textOverflow 	Sets or returns what should happen when text overflows the containing element
textShadow 	Sets or returns the shadow effect of a text
textTransform 	Sets or returns the capitalization of a text
top 	Sets or returns the top position of a positioned element
transform 	Applies a 2D or 3D transformation to an element
transformOrigin 	Sets or returns the position of transformed elements
transformStyle 	Sets or returns how nested elements are rendered in 3D space
transition 	A shorthand property for setting or returning the four transition properties
transitionProperty 	Sets or returns the CSS property that the transition effect is for
transitionDuration 	Sets or returns how many seconds or milliseconds a transition effect takes to complete
transitionTimingFunction 	Sets or returns the speed curve of the transition effect
transitionDelay 	Sets or returns when the transition effect will start
unicodeBidi 	Sets or returns whether the text should be overridden to support multiple languages in the same document
userSelect 	Sets or returns whether the text of an element can be selected or not
verticalAlign 	Sets or returns the vertical alignment of the content in an element
visibility 	Sets or returns whether an element should be visible
whiteSpace 	Sets or returns how to handle tabs, line breaks and whitespace in a text
width 	Sets or returns the width of an element
wordBreak 	Sets or returns line breaking rules for non-CJK scripts
wordSpacing 	Sets or returns the spacing between words in a text
wordWrap 	Allows long, unbreakable words to be broken and wrap to the next line
widows 	Sets or returns the minimum number of lines for an element that must be visible at the top of a page
zIndex 	Sets or returns the stack order of a positioned element
#endif
};


struct Document {
    //Returns the currently focused element in the document
    // activeElement 	

    // Returns the window object associated with a document, or null if none is available.
    // defaultView 	

    // 	Controls whether the entire document should be editable or not.
    // designMode 

    // Returns the Document Type Declaration associated with the document
    // doctype 	

    // Returns the Document Element of the document (the <html> element)
    // documentElement 	

    // Deprecated
    // documentMode 	

    // Sets or returns the location of the document
    // documentURI 	

    // Returns the domain name of the server that loaded the document
    // domain 	

    // Deprecated
    // domConfig 	

    // 	Returns a collection of all <embed> elements the document
    // embeds 
    // anchors 	Deprecated
    // applets 	Deprecated

    // Returns the absolute base URI of a document
    // baseURI 

    // Sets or returns the document's body (the <body> element)
    // body 	

    // Deprecated
    // charset 	
    // Returns the character encoding for the document
    // characterSet 	

    // Returns all name/value pairs of cookies in the document
    // cookie 	

    // Deprecated
    // execCommand() 	

    // 	Returns a collection of all <form> elements in the document
    // forms 

    //! Attaches an event handler to the document
    void addEventListener(str const& event, std::function<void()> callback, bool capture = false);

    //!  Removes an event handler from the window
    void removeEventListener(str const& event, std::function<void()> callback, bool capture = false);

    // Adopts a node from another document
    Element adoptNode(Element n); 	

    // Closes the output stream previously opened with document.open()
    void close(str const& name);

    // Creates an attribute node
    Attribute createAttribute();

    // Creates a Comment node with the specifiode 
    Element createComment(str const& text);

    // Creates an empty DocumentFragmennt node
    Element createDocumentFragment();

    // Creates an Element node
    Element createElement(str const& type); 

    // Creates a new event
    Event createEvent(str const& type);

    // Creates a Text node
    Element createTextNode(str const& text); 

    // Returns the element that has the ID attribute with the specified value
    Element getElementById(str const& id); 	

    // Returns an HTMLCollection containing all elements with the specified class name
    Collection getElementsByClassName(str const& classname); 	

    // Returns an live NodeList containing all elements with the specified name
    NodeList getElementsByName(str const& iname); 	

    // Returns an HTMLCollection containing all elements with the specified tag name
    Collection getElementsByTagName(str const& tagname);

    // Returns a Boolean value indicating whether the document has focus
    bool hasFocus(); 	

    // Imports a node from another document
    Element importNode(Element node, bool deep);	

    // Removes empty Text nodes, and joins adjacent nodes
    void normalize();

    // Opens an HTML output stream to collect output from document.write()
    void open();

    // Returns the first element that matches a specified CSS selector(s) in the document
    NodeList querySelector(str const& selector); 	

    // 	Returns a static NodeList containing all elements that matches a specified CSS selector(s) in the document
    NodeList querySelectorAll(str const& selector); 

    // Writes HTML expressions or JavaScript code to a document
    template<typename ...Args>
    void write(Args... args); 	

    // Same as write(), but adds a newline character after each statement
    template<typename ...Args>
    void writeln(Args... args); 	

    // Deprecated
    // normalizeDocument() 	


    // Deprecated
    // renameNode() 	

    // // Returns the <head> element of the document
    // head 	

    // // Returns a collection of all <img> elements in the document
    // images 	

    // // Returns the DOMImplementation object that handles this document
    // implementation 	

    // // Deprecated
    // inputEncoding 	

    // // Returns the date and time the document was last modified
    // lastModified 	

    // // Returns a collection of all <a> and <area> elements in the document that have a href attribute
    // links 	

    // // Returns the (loading) status of the document
    // readyState 	

    // // Returns the URL of the document that loaded the current document
    // referrer 	

    // // Returns a collection of <script> elements in the document
    // scripts 	

    // // Deprecated
    // strictErrorChecking 	

    // // Sets or returns the title of the document
    // title 	

    // // Returns the full URL of the HTML document
    // URL 	
};

struct Event {
    // bubbles 	Returns whether or not a specific event is a bubbling event
    // cancelBubble 	Sets or returns whether the event should propagate up the hierarchy or not
    // cancelable 	Returns whether or not an event can have its default action prevented
    // composed 	Returns whether the event is composed or not
    // createEvent() 	Creates a new event
    // composedPath() 	Returns the event's path
    // currentTarget 	Returns the element whose event listeners triggered the event
    // defaultPrevented 	Returns whether or not the preventDefault() method was called for the
    // event eventPhase 	Returns which phase of the event flow is currently being evaluated
    // isTrusted 	Returns whether or not an event is trusted
    // preventDefault() 	Cancels the event if it is cancelable, meaning that the default action that
    // belongs to the event will not occur stopImmediatePropagation() 	Prevents other listeners of
    // the same event from being called stopPropagation() 	Prevents further propagation of an
    // event during event flow target 	Returns the element that triggered the event timeStamp
    // Returns the time (in milliseconds relative to the epoch) at which the event was created type
    // Returns the name of the event
};

struct Canvas {};

// FETCH

/*
// Events
Event 	Occurs When 	Belongs To
abort 	The loading of a media is aborted 	UiEvent, Event
afterprint 	A page has started printing 	Event
animationend 	A CSS animation has completed 	AnimationEvent
animationiteration 	A CSS animation is repeated 	AnimationEvent
animationstart 	A CSS animation has started 	AnimationEvent
beforeprint 	A page is about to be printed 	Event
beforeunload 	Before a document is about to be unloaded 	UiEvent, Event
blur 	An element loses focus 	FocusEvent
canplay 	The browser can start playing a media (has buffered enough to begin) 	Event
canplaythrough 	The browser can play through a media without stopping for buffering 	Event
change 	The content of a form element has changed 	Event
click 	An element is clicked on 	MouseEvent
contextmenu 	An element is right-clicked to open a context menu 	MouseEvent
copy 	The content of an element is copied 	ClipboardEvent
cut 	The content of an element is cut 	ClipboardEvent
dblclick 	An element is double-clicked 	MouseEvent
drag 	An element is being dragged 	DragEvent
dragend 	Dragging of an element has ended 	DragEvent
dragenter 	A dragged element enters the drop target 	DragEvent
dragleave 	A dragged element leaves the drop target 	DragEvent
dragover 	A dragged element is over the drop target 	DragEvent
dragstart 	Dragging of an element has started 	DragEvent
drop 	A dragged element is dropped on the target 	DragEvent
durationchange 	The duration of a media is changed 	Event
ended 	A media has reach the end ("thanks for listening") 	Event
error 	An error has occurred while loading a file 	ProgressEvent, UiEvent, Event
focus 	An element gets focus 	FocusEvent
focusin 	An element is about to get focus 	FocusEvent
focusout 	An element is about to lose focus 	FocusEvent
fullscreenchange 	An element is displayed in fullscreen mode 	Event
fullscreenerror 	An element can not be displayed in fullscreen mode 	Event
hashchange 	There has been changes to the anchor part of a URL 	HashChangeEvent
input 	An element gets user input 	InputEvent, Event
invalid 	An element is invalid 	Event
keydown 	A key is down 	KeyboardEvent
keypress 	A key is pressed 	KeyboardEvent
keyup 	A key is released 	KeyboardEvent
load 	An object has loaded 	UiEvent, Event
loadeddata 	Media data is loaded 	Event
loadedmetadata 	Meta data (like dimensions and duration) are loaded 	Event
loadstart 	The browser starts looking for the specified media 	ProgressEvent
message 	A message is received through the event source 	Event
mousedown 	The mouse button is pressed over an element 	MouseEvent
mouseenter 	The pointer is moved onto an element 	MouseEvent
mouseleave 	The pointer is moved out of an element 	MouseEvent
mousemove 	The pointer is moved over an element 	MouseEvent
mouseover 	The pointer is moved onto an element 	MouseEvent
mouseout 	The pointer is moved out of an element 	MouseEvent
mouseup 	A user releases a mouse button over an element 	MouseEvent
mousewheel 	Deprecated. Use the wheel event instead 	WheelEvent
offline 	The browser starts working offline 	Event
online 	The browser starts working online 	Event
open 	A connection with the event source is opened 	Event
pagehide 	User navigates away from a webpage 	PageTransitionEvent
pageshow 	User navigates to a webpage 	PageTransitionEvent
paste 	Some content is pasted in an element 	ClipboardEvent
pause 	A media is paused 	Event
play 	The media has started or is no longer paused 	Event
playing 	The media is playing after being paused or buffered 	Event
popstate 	The window's history changes 	PopStateEvent
progress 	The browser is downloading media data 	Event
ratechange 	The playing speed of a media is changed 	Event
resize 	The document view is resized 	UiEvent, Event
reset 	A form is reset 	Event
scroll 	An scrollbar is being scrolled 	UiEvent, Event
search 	Something is written in a search field 	Event
seeked 	Skipping to a media position is finished 	Event
seeking 	Skipping to a media position is started 	Event
select 	User selects some text 	UiEvent, Event
show 	A <menu> element is shown as a context menu 	Event
stalled 	The browser is trying to get unavailable media data 	Event
storage 	A Web Storage area is updated 	StorageEvent
submit 	A form is submitted 	Event
suspend 	The browser is intentionally not getting media data 	Event
timeupdate 	The playing position has changed (the user moves to a different point in the media)
Event toggle 	The user opens or closes the <details> element 	Event touchcancel 	The touch is
interrupted 	TouchEvent touchend 	A finger is removed from a touch screen 	TouchEvent
touchmove 	A finger is dragged across the screen 	TouchEvent
touchstart 	A finger is placed on a touch screen 	TouchEvent
transitionend 	A CSS transition has completed 	TransitionEvent
unload 	A page has unloaded 	UiEvent, Event
volumechange 	The volume of a media is changed (includes muting) 	Event
waiting 	A media is paused but is expected to resume (e.g. buffering) 	Event
wheel 	The mouse wheel rolls up or down over an element 	WheelEvent

*/

}  // namespace js

using namespace emscripten;
using namespace lython;

// class_<SemanticAnalyser>("SemanticAnalyser")
//     .constructor<int, std::string>()
//     .function("incrementX", &MyClass::incrementX)
//     .property("x", &MyClass::getX, &MyClass::setX)
//     .class_function("getStringFromInstance", &MyClass::getStringFromInstance)
// ;

template <typename T, typename... Args>
class_<std::vector<T, Args...>> register_array(const char* name) {
    typedef std::vector<T, Args...> VecType;

#if __cplusplus >= 201703L
    register_optional<T, Args...>();
#endif

    void (VecType::*push_back)(const T&)            = &VecType::push_back;
    void (VecType::*resize)(const size_t, const T&) = &VecType::resize;
    size_t (VecType::*size)() const                 = &VecType::size;

    return class_<std::vector<T, Args...>>(name)
        .template constructor<>()
        .function("push_back", push_back)
        .function("resize", resize)
        .function("size", size)
        .function("get", &internal::VectorAccess<VecType>::get)
        .function("set", &internal::VectorAccess<VecType>::set);
}

template <typename K, typename V, typename... Args>
class_<std::map<K, V, Args...>> register_dict(const char* name) {
    typedef std::map<K, V, Args...> MapType;
#if __cplusplus >= 201703L
    register_optional<V>();
#endif

    size_t (MapType::*size)() const = &MapType::size;
    return class_<MapType>(name)
        .template constructor<>()
        .function("size", size)
        .function("get", internal::MapAccess<MapType>::get)
        .function("set", internal::MapAccess<MapType>::set)
        .function("keys", internal::MapAccess<MapType>::keys);
}

String make_string(std::string const& str) { return String(std::begin(str), std::end(str)); }

void function(emscripten::val val) {}

void fun(int arg) { std::cout << "Hello " << arg << "\n"; }

// Buffer
EMSCRIPTEN_BINDINGS(lython) {

    register_vector<int>("vector<int>");

    class_<Node>("Node");
    class_<Module>("Module");

    class_<String>("String").constructor(make_string);

    class_<StringBuffer>("StringBuffer")
        .constructor<String>()
        .function("next", &StringBuffer::getc)
        .function("current", &StringBuffer::peek);

    // = ;

    class_<Token>("Token")
        .function("line", &Token::line)
        .function("col", &Token::col)
        .function("type", &Token::type)
        .function("identifier", select_overload<String const&() const>(&Token::identifier));

    class_<Lexer>("Lexer")
        .constructor<StringBuffer&>()
        .function("next", &Lexer::next_token)
        .function("peek", &Lexer::peek_token)
        .function("current", &Lexer::token);

    // return_value_policy::reference
    class_<Parser>("Parser")
        .constructor<Lexer&>()
        .function("parse_module", &Parser::parse_module, return_value_policy::take_ownership())
        .function("next", &Parser::next, return_value_policy::reference());

    class_<SemaException>("SemaException");

    SemaVisitorTrait::ModRet (SemanticAnalyser::*method)(Module*, int) = &SemanticAnalyser::module;

    class_<SemanticAnalyser>("SemanticAnalyser")
        .constructor<>()
        .function("analyse", method, allow_raw_pointers())
        .function("has_errors", &SemanticAnalyser::has_errors);

    function("callback", fun);
    // emscripten::function("function_callback_function", &fun);
}

#include <iostream>

std::ostream& operator<<(std::ostream& out, emscripten::val& val) { return out; }

EMSCRIPTEN_DECLARE_VAL_TYPE(CallbackType);

void fun2(int arg) { std::cout << "Hello " << arg + 1 << "\n"; }

int main(int argc, const char* argv[]) {
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    //

    class_<std::function<void()>>("_callback")
        .constructor<>()
        .function("opcall", &std::function<void()>::operator());

    emscripten::val window = emscripten::val::global("window");

    static std::function<void()> wrapped = []() {
        std::cout << "Hello "
                  << "\n";
    };

    //
    // val obj = val::object();
    // obj.set("opcall", wrapped);

    // auto fun = emscripten::val::module_property("callback");

    // auto fun = emscripten::val::set("callback", fun2);

    // emscripten::val intervalId = window.call<emscripten::val>("setInterval", fun, 1000, 1);

#if 0

    js::Window w;
 
    // w.alert("I am HERE");

    w.setInterval([]() {
        std::cout << "Hello " << "\n";
    }, 1);

     w.alert("I am HERE");
#endif
    return 0;
}

namespace js {

Window::Window(emscripten::val window): window(window) {}

void Window::addEventListener(str const& event, std::function<void()> callback, bool capture) {
    window.call<void>("addEventListener", event, callback, capture);
}

void Window::removeEventListener(str const& event, std::function<void()> callback, bool capture) {
    window.call<void>("removeEventListener", event, callback, capture);
}

void Window::alert(str const& msg) { window.call<void>("alert", msg); }

bool Window::confirm(str const& msg) { window.call<void>("confirm", msg); }

str Window::prompt(str const& msg, str const& default_text) { window.call<void>("prompt", msg, default_text); }

str Window::atob(str const& base64) { window.call<str>("atob", base64); }

str Window::btoa(str const& data) { window.call<str>("btoa", data); }

emscripten::val Window::matchMedia(str const& media) { window.call<void>("matchMedia", media); }

void Window::moveBy(float x, float y) { window.call<void>("moveBy", x, y); }

void Window::moveTo(float x, float y) { window.call<void>("moveTo", x, y); }

std::optional<Window> Window::open(str const& url, str const& name, str const& specs) {
    emscripten::val w = window.call<emscripten::val>("open", url, name, specs);
    return w;
}

void Window::blur() { window.call<void>("blur"); }

void Window::focus() { window.call<void>("focus"); }

void Window::close() { window.call<void>("close"); }

void Window::print() { window.call<void>("print"); }

AnimId Window::requestAnimationFrame(std::function<void(float)> callback) { 
    return window.call<val>("requestAnimationFrame", callback); 
}

void Window::cancelAnimationFrame(AnimId id) { window.call<void>("cancelAnimationFrame", id); }

void Window::resizeBy(float x, float y) { window.call<void>("resizeBy", x, y); }

void Window::resizeTo(float x, float y) { window.call<void>("resizeTo", x, y); }

void Window::scrollBy(float x, float y) { window.call<void>("scrollBy", x, y); }

void Window::scrollTo(float x, float y) { window.call<void>("scrollTo", x, y); }

template <typename Fun, typename... Args>
TimerHandle Window::setInterval(Fun callback, float milliseconds, Args... args) {
    // register_type<Fun>("C++ function type");

    // typedef void (*em_callback_func)(void)

    std::function<void()> wrapped = [callback, args...]() { callback(args...); };

    val obj = val::object();
    obj.set("opcall", wrapped);

    //
    emscripten::val intervalId = window.call<emscripten::val>("setInterval", obj, milliseconds);

    return intervalId;
}

void Window::clearInterval(TimerHandle timer) { window.call<void>("clearInterval", timer);  }

template <typename... Args>
TimerHandle Window::setTimeout(void (*callback)(Args...), float milliseconds, Args... args) {
    // Wrap the callback and its arguments in a lambda
    auto wrappedCallback = [callback, args...]() { callback(args...); };

    emscripten::val module = emscripten::val::global("Module");
    module.set("callCallback", ([wrappedCallback]() { wrappedCallback(); }));

    // Convert the lambda into a JavaScript function
    // emscripten::val js_callback = emscripten::function(wrappedCallback);

    emscripten::val intervalId = window.call<emscripten::val>(
        "setTimeout", emscripten::val::module_property("callCallback"));

    return intervalId;
}

void Window::clearTimeout(TimerHandle timer) { window.call<void>("clearTimeout", timer); }

void Window::stop() { window.call<void>("stop"); }

//! Gets the current computed CSS styles applied to an element
// val Window::getComputedStyle(element, pseudoElement)

//! Deprecated. Use scrollTo() instead.
// Window::scroll()

//! Returns a Selection object representing the range of text selected by the user
// Window::getSelection()

/*
//! The console object provides access to the browser's debugging console.
Console* Window::console();
History* Window::history();
Location* Window::location();
Navigator* Window::navigator();
Screen* Window::screen();
*/
}  // namespace js

#endif
