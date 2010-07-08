#!/usr/bin/ruby
require 'rubygems'
require 'rmotion'

m=Motion.new

m.write= "capture.avi"

m.cam  { |e|  
	p e.first.center
	p e.first.corners

} 
