{
	'variables': {
		'chromium_code': 0,
		#'win_third_party_warn_as_error': 'false',
	},
	'includes': [
  	],
  	'targets': [
  	{
		'target_name': 'squirrel',
		'type': 'static_library',
		'variables': {'enable_wexit_time_destructors': 1,},
		'dependencies': [
		],
		'sources': [
			'squirrel/sqapi.cpp',
			'squirrel/sqbaselib.cpp',
			'squirrel/sqclass.cpp',
			'squirrel/sqcompiler.cpp',
			'squirrel/sqdebug.cpp',
			'squirrel/sqfuncstate.cpp',
			'squirrel/sqlexer.cpp',
			'squirrel/sqmem.cpp',
			'squirrel/sqobject.cpp',
			'squirrel/sqstate.cpp',
			'squirrel/sqtable.cpp',
			'squirrel/sqvm.cpp',
		],
      	'include_dirs': [
        	'include',
      	],			
		'msvs_settings': {
			'VCCLCompilerTool': {
				'WarnAsError': 'false',
			}
		},
  	},
  	{
		'target_name': 'sqstdlib',
		'type': 'static_library',
		'variables': {'enable_wexit_time_destructors': 1,},
		'dependencies': [
			'squirrel',
		],
		'sources': [
			'sqstdlib/sqstdaux.cpp',
			'sqstdlib/sqstdblob.cpp',
			'sqstdlib/sqstdio.cpp',
			'sqstdlib/sqstdmath.cpp',
			'sqstdlib/sqstdrex.cpp',
			'sqstdlib/sqstdstream.cpp',
			'sqstdlib/sqstdstring.cpp',
			'sqstdlib/sqstdsystem.cpp',
		],
      	'include_dirs': [
        	'include',
      	],		
		'msvs_settings': {
			'VCCLCompilerTool': {
				'WarnAsError': 'false',
			}
		},
  	},
  	{
		'target_name': 'sqshell',
		'type': 'executable',
		'variables': {'enable_wexit_time_destructors': 1,},
		'dependencies': [
			'squirrel',
			'sqstdlib',
		],
		'sources': [
			'sq/sq.c',
		],
      	'include_dirs': [
        	'include',
      	],			
		'msvs_disabled_warnings': [4267, ],
		'msvs_settings': {
			'VCLinkerTool': {
				#'SubSystem': '2',         # Set /SUBSYSTEM:WINDOWS
			},
			'VCCLCompilerTool': {
				'WarnAsError': 'false',
			}
		},
  	}],
 }