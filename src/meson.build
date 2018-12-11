gnome = import('gnome')

sources = [
'main.c',         
'uart-config.c',  
'uart-read.c',     
'uart-send.c',   
'uart-text.c',
'uart-bottom.c',  
'uart-init.c',    
'uart-receive.c',  
'uart-share.c',  
'uart-write.c',
]

executable('uart',
  sources : sources,
  dependencies : [gtk_deps,vte_deps],
  include_directories: top_srcdir,
  install : true,
  install_dir : get_option('bindir')
  )
