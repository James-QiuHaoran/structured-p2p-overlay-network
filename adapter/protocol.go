

package p2p

import (
	"fmt"

	"github.com/ethereum/go-ethereum/p2p/enode"
	"github.com/ethereum/go-ethereum/p2p/enr"
)

type Protocol struct {
	Name string
	Version uint
	Length uint64
	Run func(peer *Peer, rw MsgReadWriter) error
	NodeInfo func() interface{}
	PeerInfo func(id enode.ID) interface{}
	Attributes []enr.Entry
}

func (p Protocol) cap() Cap {
	return Cap{p.Name, p.Version}
}

// Cap is the structure of a peer capability.
type Cap struct {
	Name    string
	Version uint
}

func (cap Cap) String() string {
	return fmt.Sprintf("%s/%d", cap.Name, cap.Version)
}

type capsByNameAndVersion []Cap

func (cs capsByNameAndVersion) Len() int      { return len(cs) }
func (cs capsByNameAndVersion) Swap(i, j int) { cs[i], cs[j] = cs[j], cs[i] }
func (cs capsByNameAndVersion) Less(i, j int) bool {
	return cs[i].Name < cs[j].Name || (cs[i].Name == cs[j].Name && cs[i].Version < cs[j].Version)
}

func (capsByNameAndVersion) ENRKey() string { return "cap" }

type Msg struct {
	Code       uint64
	Size       uint32 // size of the paylod
	Payload    io.Reader
	ReceivedAt time.Time
}

func (msg Msg) String() string {
	return fmt.Sprintf("msg #%v (%v bytes)", msg.Code, msg.Size)
}

func (p Protocol) broadcast(m Msg) error{
	err := p.MsgReadWriter.WriteMsg(msg)
	if err != nil {
		return err
	}
	p.feed.Send(&PeerEvent{
		Type:     PeerEventTypeMsgSend,
		Peer:     ev.peerID,
		Protocol: ev.Protocol,
		MsgCode:  &msg.Code,
		MsgSize:  &msg.Size,
	})
	
	return nil
}

