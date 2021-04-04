package main

import "fmt"

func main() {
	a := 0
	defer fmt.Println(a)
	a = 1
}
