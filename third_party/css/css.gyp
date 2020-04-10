#css parse
{
	'targets': [
		{
			'target_name':'css',
			'type':'static_library',
			'sources':[
				'css_document.h',
				'css_document.cpp',
				'css_document_impl.h',
				'css_document_impl.cpp',
				'css_property.cpp',
				'css_property.h',
				'css_property_impl.h',
				'css_ruleset.h',
				'css_selector.h',
				'css_selector.cpp',
				'css_token.h',
				'css_tokenizer.cpp',
				'css_tokenizer.h',
				'css_tokenizer_impl.cpp',
				'css_tokenizer_impl.h',
				'css_export.h',
			],
			'include_dirs': [
        		'.',
        		'../',
        		'../../',
      		],
      		'defines': [
        		'CSS_IMPLEMENTATION',
      		],
      		'direct_dependent_settings': {
        		'include_dirs': [
          			'.',
        		],
      		},
		},
	],
}