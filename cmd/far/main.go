package main

import "log"

func main() {
	app := new(App)
	/*__*/ if err := app.New(); err != nil {
		log.Fatalln("new: ", err)
	} else if err := app.Run(); err != nil {
		log.Fatalln("run: ", err)
	}
}
