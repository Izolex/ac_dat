package main

import (
	"ac_dat_client"
	"bufio"
	"encoding/binary"
	"flag"
	"fmt"
	"os"
	"time"
)

const defaultSearchMode = ac_dat_client.SearchModeFirst | ac_dat_client.SearchModeNeedle | ac_dat_client.SearchModeUserData

var flagEndian = flag.String("e", "little", "endian (little,big)")
var flagNetwork = flag.String("n", "tcp", "network (unix|tcp)")
var flagAddress = flag.String("a", "127.0.0.1:80", "address (ip|socket path)")
var flagSearchMode = flag.Int("sm", defaultSearchMode, "search mode")

func printOccurrence(mode ac_dat_client.SearchMode, occurrence *ac_dat_client.Occurrence) {
	for occurrence != nil {
		fmt.Print("found")
		if 0 < mode&ac_dat_client.SearchModeNeedle {
			fmt.Printf(" \"%s\" (%d)", occurrence.Needle, occurrence.NeedleLength)
		}
		if 0 < mode&ac_dat_client.SearchModeUserData {
			fmt.Printf(" data length: \"%s\" %d", occurrence.DataValue, occurrence.DataSize)
		}
		fmt.Println()
		occurrence = occurrence.Next
	}
}

func getEndian() binary.ByteOrder {
	if *flagEndian == "little" {
		return binary.LittleEndian
	} else {
		return binary.BigEndian
	}
}

func main() {
	flag.Parse()

	c, err := ac_dat_client.NewClient(getEndian(), *flagNetwork, *flagAddress)
	if err != nil {
		panic(err)
	}
	defer c.Close()

	zeroTime := time.Time{}
	mode := ac_dat_client.SearchMode(*flagSearchMode)

	for {
		fmt.Print("Needle: ")
		reader := bufio.NewReader(os.Stdin)
		text, err := reader.ReadString('\n')
		if err != nil {
			panic(err)
		}

		err = c.Search(text[:len(text)-1], mode, zeroTime)
		if err != nil {
			panic(err)
		}

		occurrence, err := c.Occurrence(mode, zeroTime)
		if err != nil {
			panic(err)
		}
		if occurrence == nil {
			fmt.Println("not found")
		} else {
			printOccurrence(mode, occurrence)
		}
	}
}
