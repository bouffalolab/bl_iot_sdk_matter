# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))
import sphinx_rtd_theme
import os
import sys 
import importlib
importlib.reload(sys)
#reload(sys)
#sys.setdefaultencoding('utf-8')

# -- Project information -----------------------------------------------------

project = 'BL602 IoT SDK'
copyright = '2020, Bouffalo Lab'
author = 'Bouffalo Lab'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.imgmath',
    'sphinx_rtd_theme',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']
source_suffix = '.rst'

master_doc = 'index'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = 'zh'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
html_logo = 'content/logo.png'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_css_files = [
    'css/custom.css',
]

def setup(app):
    app.add_stylesheet("style.css")


latex_engine = 'xelatex'
latex_elements = {
    'fontenc': '\\usepackage{fontspec}',
    'fontpkg': '''\
\\usepackage[table,dvipsnames]{xcolor}
\\usepackage[utf8]{inputenc} 
\\usepackage[T1]{fontenc} 
\\usepackage{stix} 
\\usepackage[noindent,UTF8]{ctexcap}
\\usepackage[colorlinks,urlcolor=black,linkcolor=MidnightBlue]{hyperref}
\\setmainfont{Arial}
\\setCJKmainfont{SimSun}''',
    'geometry': '\\usepackage[left=1.6cm, right=1.6cm,top=2.5cm,bottom=2cm]{geometry}',
    'preamble': '''\
\\usepackage[titles]{tocloft}
\\usepackage{xeCJK} 
\\usepackage{hyphenat}


\\setlength{\\parskip}{4pt}   
\\linespread{1.6}
\\setlength\\leftskip{73pt}   

\\usepackage{chngcntr}
\\counterwithin{figure}{chapter}


\\usepackage{titlesec}
\\usepackage{ctex}
\CTEXsetup[name={,}, number=\\arabic{chapter}]{chapter}

\makeatletter
\\renewcommand{\@seccntformat}[1]{\llap{\color{MidnightBlue}{\csname the#1\endcsname}\hspace{35pt}}}                    
\\renewcommand{\section}{\@startsection{section}{1}{\z@}
	{1ex \@plus 5ex \@minus 5ex}
	{1ex \@plus 5ex }
	{\zihao{4}\color{MidnightBlue}\heiti\\bfseries}}
\\renewcommand{\subsection}{\@startsection {subsection}{2}{\z@}
	{0.5ex \@plus -0.1ex \@minus -.4ex}
	{0.5ex \@plus.2ex }
	{\zihao{-4}\color{MidnightBlue}\heiti\\bfseries}}

\\newcommand\\bolddef[1]{\zihao{-4}\heiti\\bfseries #1}

%%%%%%%%%%liebiao%%%%%%%%%%%%%%
\\usepackage{indentfirst}
\\usepackage{enumitem}
\\setlist[itemize,1]{
leftmargin=85pt
} 

\\setlist[itemize,2]{
	leftmargin=12pt,
} 

\\setlist[enumerate,1]{
	leftmargin=85pt
} 

\\setlist[enumerate,2]{
	leftmargin=12pt,
} 

\\usepackage{fancyhdr}
\\pagestyle{fancy}
\\fancyhf{}
\\cfoot{\\thepage}
\\lfoot{博流智能科技}
\\fancyhead[L]{\\rightmark}

\\pagenumbering{arabic}
   
\\renewcommand\\arraystretch{1.3}
\\usepackage{colortbl}
\\newfloat{flowchart}{H}{loflow}
\\floatstyle{plaintop}
\\restylefloat{flowchart}
\\usepackage{float}

\\usepackage{caption}
\\newcommand\\smallpic[2]{
\\begin{figure}[H]
	\\hspace{73pt}
%	\\setlength{\\abovecaptionskip{0pt}}
%	\\setlength{\\belowcaptionskip{0pt}}
	\\captionsetup{
    justification=centering,
    singlelinecheck=false,
    margin={73pt,0pt}	
}
\\includegraphics{#2}
\\caption{#1}
%\\label{#3}
\\end{figure}
}

\\newcommand\\regover[1]
{
	\\begin{center}
		\\begin{longtable}[H]{|p{140pt}|p{350pt}|}
			\hline\hline
			\\rowcolor{gray!40}
			\\begin{minipage}{1.5cm}\\vspace{0.2cm}{\\bfseries 名称}\\vspace{0.2cm}\\end{minipage}&\\bfseries 描述 \\\\
			
			\hline\hline
			\endhead
			\hline\hline
			\endlastfoot
			
			
		#1
		\\end{longtable}					  							
	\\end{center}	
}

\\newcommand\\regbit[1]
{
	\\begin{table}[H]
	    \\footnotesize
		\\begin{tabular}[c]{p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}p{20.7pt}}
			
			
			
			#1
		\\end{tabular}					  							
	\\end{table}			
}

\\newcommand\\regdes[1]
{
	\\begin{center}
	\\small
		\\rowcolors{2}{gray!40}{gray!20}
		\\begin{longtable}[c]{p{40pt}p{65pt}p{70pt}p{300pt}}
			\hline\hline
			\\begin{minipage}{1.5cm}\\vspace{0.2cm}{\\bfseries 位}\\vspace{0.2cm}\end{minipage}&\\bfseries 名称&\\bfseries 复位值&\\bfseries 描述 \\\\
			
			\hline\hline
			\endhead
			\hline\hline
			\endlastfoot
			
			
			#1
			
			\hline
		\\end{longtable}					  							
	\\end{center}	
	\\vspace{0.5em}	
}

% 目录
\\usepackage{shorttoc}


\\usepackage{graphicx}
\\hyphenpenalty=5
\\tolerance=1
\\usepackage{ragged2e}


\\usepackage{array}
\\newcommand{\\tabincell}[2]{\\begin{tabular}{@{}#1@{}}#2\end{tabular}}  %表格自动换行
\\cftsetpnumwidth{1.0cm}\\cftsetrmarg{1.2cm}
\\setlength{\\cftchapnumwidth}{0.75cm}
\\setlength{\\cftsubsecnumwidth}{1.5cm}
\\setlength{\\cftsecindent}{\\cftchapnumwidth}
\\setlength{\\cftsecnumwidth}{1cm}''',
    'fncychap': '\\usepackage[Bjornstrup]{fncychap}',
    'printindex': '\\footnotesize\\raggedright\\printindex',
}
#latex_show_urls = 'footnote'

