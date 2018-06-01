{
    "targets": [
        {
            "target_name": "nativeclipper",
            "sources": [
                "nativeclipper.cc",
                "../CPP/clipper.cpp",
            ],
            "include_dirs" : [
                "<!(node -e \"require('nan')\")"
            ],
            "cflags!": [ "-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "conditions": [
                [
                    'OS=="mac"',
                    {
                        'xcode_settings':
                        {
                            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                        }
                    }
                ]
            ]    
        }
    ],
}

