package parhash

import (
	"context"
	"time"
		
	"net"
	"sync"
	
	"github.com/pkg/errors"
	"github.com/prometheus/client_golang/prometheus"
	"google.golang.org/grpc"
	hashpb "fs101ex/pkg/gen/hashsvc"
	"golang.org/x/sync/semaphore"

	parhashpb "fs101ex/pkg/gen/parhashsvc"

	"fs101ex/pkg/workgroup"




)

type Config struct {
	ListenAddr   string
	BackendAddrs []string
	Concurrency  int

	Prom prometheus.Registerer
}

// Implement a server that responds to ParallelHash()
// as declared in /proto/parhash.proto.
//
// The implementation of ParallelHash() must not hash the content
// of buffers on its own. Instead, it must send buffers to backends
// to compute hashes. Buffers must be fanned out to backends in the
// round-robin fashion.
//
// For example, suppose that 2 backends are configured and ParallelHash()
// is called to compute hashes of 5 buffers. In this case it may assign
// buffers to backends in this way:
//
//	backend 0: buffers 0, 2, and 4,
//	backend 1: buffers 1 and 3.
//
// Requests to hash individual buffers must be issued concurrently.
// Goroutines that issue them must run within /pkg/workgroup/Wg. The
// concurrency within workgroups must be limited by Server.sem.
//
// WARNING: requests to ParallelHash() may be concurrent, too.
// Make sure that the round-robin fanout works in that case too,
// and evenly distributes the load across backends.
//
// The server must report the following performance counters to Prometheus:
//
//  1. nr_nr_requests: a counter that is incremented every time a call
//     is made to ParallelHash(),
//
//  2. subquery_durations: a histogram that tracks durations of calls
//     to backends.
//     It must have a label `backend`.
//     Each subquery_durations{backed=backend_addr} must be a histogram
//     with 24 exponentially growing buckets ranging from 0.1ms to 10s.
//
// Both performance counters must be placed to Prometheus namespace "parhash".
type Server struct {
	conf Config
	MutexSyncronizer sync.Mutex
	checker int
	previous int
	
	nr_nr_requests prometheus.Counter 
	subquery_durations *prometheus.HistogramVec

	stop context.CancelFunc
	l    net.Listener
	wg   sync.WaitGroup
	
	sem *semaphore.Weighted
}

func New(conf Config) *Server {
	return &Server{
		conf: conf,
		sem:  semaphore.NewWeighted(int64(conf.Concurrency)),
		nr_nr_requests: prometheus.NewCounter(prometheus.CounterOpts
					       {
			Namespace: "parhash",
			Name: "nr_requests",
			Help: "Number of requests",
		}),
		subquery_durations: prometheus.NewHistogramVec(prometheus.HistogramOpts
							       {
			Namespace: "parhash",
			Name: "subquery_durations",
			Help: "Subquery durations",
			Buckets: prometheus.ExponentialBuckets(0.1, 1e4, 24),
		},
							       []string{"backend"}
							      ),
		checker: 0,
	}
}
//copy from hash file
func (s *Server) Start(ctx context.Context) (err error) {
	defer func() { err = errors.Wrapf(err, "Start()") }()

	ctx, s.stop = context.WithCancel(ctx)

	s.l, err = net.Listen("tcp", s.conf.ListenAddr)
	if err != nil {
		return err
	}
	srv := grpc.NewServer()
	parhashpb.RegisterParallelHashSvcServer(srv, s)
	/* We can initialize new here as:
	s.nr_nr_requests = prometheus.NewCounter(prometheus.CounterOpts{
			Namespace: "parhash",
			Name: "nr_requests",
			Help: "Number of requests",
	})
	
	s.subquery_durations = prometheus.NewHistogramVec(prometheus.HistogramOpts{
			Namespace: "parhash",
			Name: "subquery_durations",
			Help: "Subquery durations",
			Buckets: prometheus.ExponentialBuckets(0.1, 1e4, 24),}, []string{"backend"},)
	
	and keep this function below without changing 
	but that way makes warning on my system
	
	func New(conf Config) *Server {
	return &Server{
		conf: conf,
		sem:  semaphore.NewWeighted(int64(conf.Concurrency)),
	}
}
	
	*/
	
	s.conf.Prom.MustRegister(s.nr_nr_requests)
	s.conf.Prom.MustRegister(s.subquery_durations)

	s.wg.Add(2)
	go func() {
		defer s.wg.Done()

		srv.Serve(s.l)
	}()
	go func() {
		defer s.wg.Done()

		<-ctx.Done()
		s.l.Close()
	}()

	return nil
}
//zero IQ cope from hash file code

func (s *Server) ListenAddr() string {
	return s.l.Addr().String()
}
//zero IQ cope from hash file code
func (s *Server) Stop() {
	s.stop()
	s.wg.Wait()
}
// let's take that code from 11-grpc task
func (s *Server) ParallelHash(ctx context.Context, req *parhashpb.ParHashReq) (resp *parhashpb.ParHashResp, err error) {
	defer func() { err = errors.Wrapf(err, "ParallelHash()") }()
	
	s.nr_nr_requests.Inc()
	
	clients := make([]hashpb.HashSvcClient, len(s.conf.BackendAddrs))
	joins := make([]*grpc.ClientConn, len(s.conf.BackendAddrs))
	
	for i, addr := range s.conf.BackendAddrs {	
		joins[i], err = grpc.Dial(s.conf.BackendAddrs[i], grpc.WithInsecure())
		if err != nil {
			return nil, err
		}
		defer joins[i].Close()
		clients[i] = hashpb.NewHashSvcClient(joins[i])
	}
	var
	(
	workgroup1 = workgroup.New(workgroup.Config{Sem : s.sem})
	 hashes = make([][]byte, len(req.Data))
	)
	for i := range req.Data {
		number := i
		workgroup1.Go(ctx, func(ctx context.Context) error {
			var err error
			s.MutexSyncronizer.Lock()
			index := s.checker
			previous := s.checker
			s.checker = (s.checker + 1) % len(s.conf.BackendAddrs)
			if s.checker >= len(s.conf.BackendAddrs) {
				s.checker = 0
			}
			s.MutexSyncronizer.Unlock()
			
			from1 := time.Now()
			resp, err := clients[previous].Hash(ctx, &hashpb.HashReq{Data: data})
			tp1 := time.Since(from1)
			
			if err != nil {
				return err
			}
			s.subquery_durations.WithLabelValues(s.conf.BackendAddrs[index]).Observe(tp1.Seconds())
			
			s.MutexSyncronizer.Lock()
			hashes[i] = resp.Hash
			s.MutexSyncronizer.Unlock()

			return nil
		})
	}
	if err := workgroup1.Wait(); err != nil {
		return nil, err
	}
	return &parhashpb.ParHashResp{Hashes: hashes}, nil

}
