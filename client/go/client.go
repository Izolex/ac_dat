package ac_dat_client

import (
	"bytes"
	"encoding/binary"
	"errors"
	"io"
	"net"
	"time"
)

type SearchMode uint8

const (
	SearchModeFirst    = 0b00000001
	SearchModeNeedle   = 0b00000010
	SearchModeUserData = 0b00000100

	CharacterTypeSize    = 4
	UserDataSizeTypeSize = 4
)

type UserDataSize int32

type Occurrence struct {
	Next         *Occurrence
	DataSize     UserDataSize
	DataValue    []byte
	Needle       []byte
	NeedleLength int32
}

type client struct {
	connection net.Conn
	endian     binary.ByteOrder
}

func errHandler(err *error) {
	if r := recover(); r != nil {
		*err = r.(error)
	}
}

func try[T any](val T, err error) T {
	if err != nil {
		panic(err)
	}
	return val
}

func NewClient(endian binary.ByteOrder, network, address string) (*client, error) {
	connection, err := net.Dial(network, address)
	if err != nil {
		return nil, err
	}

	return &client{connection, endian}, nil
}

func (c *client) Close() error {
	return c.connection.Close()
}

func (c *client) Search(message string, mode SearchMode, timeout time.Time) (err error) {
	defer errHandler(&err)

	buffer := new(bytes.Buffer)

	c.bufferWrite(buffer, uint8(mode))
	c.bufferWrite(buffer, uint32(len(message)))
	c.bufferWrite(buffer, []byte(message))

	if !timeout.IsZero() {
		try(0, c.connection.SetWriteDeadline(timeout))
	}
	try(c.connection.Write(buffer.Bytes()))

	return
}

func (c *client) Occurrence(mode SearchMode, timeout time.Time) (firstOccurrence *Occurrence, err error) {
	defer errHandler(&err)

	if !timeout.IsZero() {
		try(0, c.connection.SetWriteDeadline(timeout))
	}

	var length UserDataSize
	var occurrence, lastOccurrence *Occurrence

	c.read(UserDataSizeTypeSize, &length)

	for 0 <= length {
		occurrence = &Occurrence{DataSize: length}

		if 0 < occurrence.DataSize {
			occurrence.DataValue = make([]byte, occurrence.DataSize)
			c.read(int32(occurrence.DataSize), occurrence.DataValue)
		}

		if 0 < mode&SearchModeNeedle {
			c.read(CharacterTypeSize, &occurrence.NeedleLength)
			occurrence.Needle = make([]byte, occurrence.NeedleLength*4)
			c.read(occurrence.NeedleLength*CharacterTypeSize, occurrence.Needle)
		}

		if firstOccurrence == nil {
			firstOccurrence = occurrence
			lastOccurrence = occurrence
			if 0 < mode&SearchModeFirst {
				break
			}
		} else {
			lastOccurrence.Next = occurrence
		}

		lastOccurrence = occurrence
		c.read(UserDataSizeTypeSize, &length)
	}

	return
}

func (c *client) read(length int32, output any) {
	data := make([]byte, length)
	c.readBytes(length, data)

	try(0, binary.Read(bytes.NewReader(data), c.endian, output))
}

func (c *client) bufferWrite(buffer io.Writer, data any) {
	try(0, binary.Write(buffer, c.endian, data))
}

func (c *client) readBytes(length int32, output []byte) {
	num := try(c.connection.Read(output))
	if int32(num) != length {
		panic(errors.New("read less bytes"))
	}
}
