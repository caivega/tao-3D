/**
 * @~english
 * @taomoduledescription{Slides, Themes for slides}
 *
 * Commands and themes to write a slide deck.
 *
 * The Slides module defines commands that you can use to define the structure
 * of your presentations: @ref slide, @ref title_slide, @ref base_slide,
 * @ref title_only_slide and more. You can write text bullets with symbols
 * like @ref ＊, @ref ‐ and  @ref ＋. \n
 *
 * The slide layout and colors are grouped by themes. Each theme as a name,
 * for instance @c "Keyboard" or @c "WhiteOnGray". Selecting a theme is just the
 * matter of using the @ref theme function with the suitable value, before using
 * the slide commands. For example:
 @code
import Slides 1.0

// Show a title slide with the default theme (black on white)
title_slide "My title slide",
    text "This shows the default theme"
// Change theme to "Keyboard"
theme "Keyboard"
slide "The Keyboard theme",
 *    * "Has a background picture"
 *    * "Is nicer than the default theme"
 @endcode
 *
 * The Slides module currently defines five themes in addition to the default
 * one, which is black text on white backround and can be selected by setting
 * @ref theme to any value that is not a valid theme name (for instance,
 * theme "Default" or, more simply, theme ""). See @ref theme for
 * details.
 *
 * In addition to these themes, the module exposes an interface so that
 * you can customize the appearance of your slides (font family, size and
 * weight; bullets; text backround; colors...)
 * @todo Document this interface.
 *
 * Slides are automatically scaled to the size of the display area.
 * Background pictures are automatically scaled to cover at least the display
 * area. A part of the background may disappear if the aspect ratio of the
 * display area is not the same as the aspect ratio of the picture.
 * All elements on the slide are drawn assuming that the slide is 1024x768
 * pixels by default. This can be adjusted with @ref set_slide_size.
 * Use @ref window_width and @ref window_height to get the size of the
 * display area. Use @ref slide_width and @ref slide_height to get the size
 * of the slide.
 *
 * Here is a more complete example showing all the slide types for some
 * themes in this module.
 * @include slides.ddd
 *
 * Here is a mosaic of all the pages generated by the above program:
 * @image html slides.png "Output from slides.ddd"
 *
 * @endtaomoduledescription{Slides}
 *
 * @~french
 * @taomoduledescription{Slides, Thèmes de présentations}
 *
 * Commandes et thèmes pour écrire des présentations.
 *
 * Le module Slides module fournit des commandes que vous pouvez utiliser pour
 * structurer vos documents : @ref slide, @ref title_slide, @ref base_slide,
 * @ref title_only_slide et d'autres. Les puces de texte peuvent être affichées
 * en utilisat @ref ＊, @ref ‐ et  @ref ＋. \n
 *
 * Les couleurs et mises en pages sont regroupées par thèmes. Chaque thème a un
 * nom, par exemple @c "Keyboard" or @c "WhiteOnGray". L'utilisation se fait
 * grâce à la commande @ref theme. Par exemple :
 @code
import Slides

// Diapositive de titre, style par défaut (black on white)
title_slide "Diapo de titre",
    text "Il s'agit du thème par défaut"
// Activation du thème "Keyboard"
theme "Keyboard"
slide "Le thème Keyboard",
 *    * "A une image de fond"
 *    * "Est plus beau que le thème par défaut"
 @endcode
 *
 * Le module Slides contient cinq thèmes en plus du défaut, qui est noir sur
 * fond blanc et qui peut être activé en utilisant <tt>theme ""</tt>. Cf. la
 * commande @ref theme pour plus de détails.
 *
 * Vous pouvez également modifier les thème, pour les adapter à vos besoins.
 * Il est possible de changer les polices de caractères, la taille du texte,
 * les puces, l'image de fond, les couleurs, etc.).
 *
 * Les diapositives peuvent se redimensionner automatiquement pour remplir
 * au mieux l'écran. Lorsque l'image de fond n'a pas le même rapport de
 * largeur et de hauteur que l'écran, une partie du fond est coupée de sorte
 * que l'image couvre quand même tout l'écran.
 * Par défaut tous les éléments d'une diapositive sont dessinés pour une
 * résolution de 1024x768 pixels. Ceci peut être ajusté en appelant
 * @ref set_slide_size. Utlisez @ref window_width et @ref window_height pour
 * obtenir la taille de l'affichage. Utilisez @ref slide_width et
 * @ref slide_height pour obtenir la taille de la diapositive.
 *
 * Voici un exemple plus complet qui illustre différents thèmes de ce
 * module.
 * @include slides.ddd
 *
 * Voici une mosaïque des pages affichées par cet exemple :
 * @image html slides.png "Thèmes et mises en page de diapositives"
 *
 * @endtaomoduledescription{Slides}
 *
 * @~
 * @{
 */



/**
 * @english
 * The name of the current theme.
 * The next command that creates a slide page will take into account the
 * value of this variable to select the appearance of the slide.
 * The possible values are:
 *  - "WhiteOnBlack" The default theme  with black and white reversed
 *  - "WhiteOnGray" Similar to the default theme, with white text on a gray
 *     backround
 *  - "WhiteOnPicture" and "BlackOnPicture" use a custom background picture
 *  - "Rounded" Similar to the default, with the addition of a thin black
 *     rounded rectangle around text areas
 *  - "Keyboard" A slightly more sophisticated theme, with a background
 *     picture and semi-transparent white text boxes
 *  - "Seyes" A theme that uses a handwriting font on a backround that looks
 *     like french ruled ("seyes") paper
 * @french
 * Le nom du thème courant.
 * La prochaine commande qui crée une diapositive utilisera la valeur de cette
 * variable pour sélectionner l'apparence de la page.
 * Les valeurs possibles sont :
 *  - "WhiteOnBlack" Texte banc sur fond noir
 *  - "WhiteOnGray" Texte blanc sur fond gris
 *  - "WhiteOnPicture" Texte blanc sur fond d'image
 *  - "BlackOnPicture" Texte noir sur fond d'image
 *  - "Rounded" Texte noir sur fond blanc, encadrés à bord arrondis
 *  - "Keyboard" Image de fond représentant un clavier, boîtes de texte
 *     semi-transparentes.
 *  - "Seyes" Utilisation d'une police de caractères qui ressemble à l'écriture
 *     manuelle, et image de fond qui représente du papier quadrillé (Séyès).
 */
text theme = "";

/**
 * @~english
 * The width of the current slide.
 * @~french
 * La largeur de la diapositive courante.
 */
real slide_width = 1024.0;

/**
 * @~english
 * The height of the current slide.
 * @~french
 * La hauteur de la diapositive courante.
 */
real slide_height = 768.0;


/**
 * @~english
 * Select a new theme for all subsequent slides
 * The @a Theme is the name of the new theme to be applied.
 * @~french
 * Choisit un nouveau thème pour les dispositives à suivre.
 * @~
 * @see theme
 */
theme(Theme:text);


/**
 * @~english
 * Creates a slide without a title and story (only the background).
 * @a P is the page name.
 * @~french
 * Crée une diapositive vide.
 * @a P is the page name.
 */
base_slide(P:text, Body:code);

/**
 * @~english
 * Creates a slide with title and picture content.
 * The slide title @a T is used as the page name.
 * @~french
 * Crée une diapositive avec un titre et une zone libre.
 * Le titre de la diapositive, @p T, est utilisé comme nom de page.
 */
picture_slide(T:text, Body:code);

/**
 * @~english
 * Creates a slide with title and content.
 * The slide title @a T is used as the page name.
 * @~french
 * Crée une diapositive avec un titre et du contenu.
 * Le titre de la diapositive, @p T, est utilisé comme nom de page.
 */
slide(T:text, Body:code);

/**
 * @~english
 * Creates a slide with title and content.
 * @a P is the page name, @a T is the slide title.
 * @~french
 * Crée une diapositive avec un titre et du contenu.
 * @p P est le nom de la page, @p T est le titre de la diapositive.
 */
named_slide(P:text, T:text, Body:code);

/**
 * @~english
 * Creates a title slide (one unique text area).
 * @a P is the page name.
 * @~french
 * Crée une diapositive de titre (une seule zone de texte).
 * @p P est le nom de la page.
 */
title_slide(P:text, Body:code);

/**
 * @~english
 * Creates a slide with a title box only.
 * The slide title @a T is used as the page name.
 * @~french
 * Crée une diapositive avec seulement un titre.
 * Le titre de la diapositive, @p T, est utilisé comme nom de page.
 */
title_only_slide(T:text, Body:code);

/**
 * @~english
 * Creates a slide with a title box only.
 * @a P is the page name, @a T is the slide title.
 * @~french
 * Crée une diapositive avec seulement un titre.
 * @p P est le nom de la page, @p T est le titre de la diapositive.
 */
title_only_slide(P:text, T:text, Body:code);

/**
 * @~english
 * Display a first-level bullet.
 * @note This is really a normal asterisk character (*). In this
 * documentation we are using a special unicode character due to
 * technical constraints.
 * @~french
 * Affiche une puce de texte de premier niveau.
 * @note Il s'agit d'une astérisque normale (*). Dans cette documentation
 * nous utilisons un caractère spécial pour des raisons techniques.
 */
＊(T:text);

/**
 * @~english
 * Display a second-level bullet.
 * Note that this is really a normal minus character (-). In this
 * documentation we are using a special unicode character due to
 * technical constraints.
 * @~french
 * Affiche une puce de texte de deuxième niveau.
 * @note Il s'agit d'un caractère moins normal (-). Dans cette documentation
 * nous utilisons un caractère spécial pour des raisons techniques.
 */
‐(T:text);

/**
 * @~english
 * Display a third-level bullet.
 * Note that this is really a normal plus character (+). In this
 * documentation we are using a special unicode character due to
 * technical constraints.
 * @~french
 * Affiche une puce de texte de troisième niveau.
 * @note Il s'agit d'un caractère plus normal (+). Dans cette documentation
 * nous utilisons un caractère spécial pour des raisons techniques.
 */
＋(T:text);

/**
 * @~english
 * Set the background picture.
 * Select @a File as the background picture. This may override the default
 * selected by @ref theme.
 * @~french
 * Définit l'image de fond.
 * Utilise @p File comme image de fond. Permet de remplacer l'image
 * par défaut sélectionnée par @ref theme.
 */
set_picture_background(File:text);

/**
 * @~english
 * Set the dimension of the slide.
 * @p Width and @p Height are expressed in pixels.
 * @~french
 * Définit la dimension de la diapositive.
 * @p Width est la largeur et @p Height la hauteur, en pixels.
 */
set_slide_size(Width:real, Height:real);

/**
 * @}
 */
