package main

import "github.com/lxn/walk"

func RGB2BGR(c uint32) walk.Color {
	return walk.Color((c&0xff0000)>>16 | (c & 0x00ff00) | (c&0x0000ff)<<8)
}
