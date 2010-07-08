#!/usr/bin/ruby
require 'rubygems'
require 'rmotion'

m=Motion.new
m.fill= false
m.rect= true

count = 0
status = Entity.status

m.vid("capture_1.avi") do |e|

  Entity.reorder status, e

  t = status.select { |u| not u.center.nil? }.count
  if t > count 
    puts "#{t - count} new " + (t > 1 ? "entities" : "entity") + " found"
  elsif t < count
    puts "#{count - t} " + (t > 1 ? "entities" : "entity") + " disappeared"
    m.point = true
    m.rect=false
  end
  count = t

  status.each_with_index do |s,n|
    puts "Entity #{n} is in position #{s.center.inspect}" unless s.center.nil?
  end

  sleep 0.10
  
end
