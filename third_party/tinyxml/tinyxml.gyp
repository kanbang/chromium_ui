#css parse
{
	'targets': [
		{
			'target_name':'tinyxml',
			'type':'static_library',
			'sources':[
				'tinystr.h',
				'tinystr.cpp',
				'tinyxml.cpp',
				'tinyxml.h',
				'tinyxmlerror.cpp',
				'tinyxmlparser.cpp',
			],
			'include_dirs': [
        		'.',
        		'../',
        		'../../',
      		],
      		'defines': [
      		],
      		'direct_dependent_settings': {
        		'include_dirs': [
          			'.',
        		],
      		},
		},
	],
}