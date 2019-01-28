{
  "targets": [
    {
      "target_name": "aio",
      "sources": [
        "init.cc"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "cflags": [
        "-std=c++11"
      ],
      "cflags_cc!": [
        "-std=gnu++0x" # Allow -std=c++14 to prevail
      ]
    }
  ]
}
