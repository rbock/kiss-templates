" Vim syntax file
" Language: Kiss-Templates

if exists("b:current_syntax")
  finish
endif

syn match basicLanguageKeywords "\$\(class\|endclass\|member\|\$\||\)"
syn match cpp "^ *%.*$"
syn match cpp "^ *%.*$"
syn region escape oneline start='\${' end='\}'
syn region raw oneline start='\$raw{' end='\}'
syn region call oneline start='\$call{' end='\}'

hi def link cpp      Comment
hi def link basicLanguageKeywords       Keyword
hi def link escape       Special
hi def link raw       Special
hi def link call       Special

