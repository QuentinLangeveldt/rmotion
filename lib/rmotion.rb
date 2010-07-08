require 'rmotion.so'

class Entity

  def initialize
    @tlx = @tly = @brx = @bry = nil
    @@radius = 60    
  end

  def center
    [(@tlx+@brx)/2,(@tly+@bry)/2] unless @tlx.nil?
  end
  
  def corners
    [@tlx,@tly,@brx,@bry]
  end

  def compare oth
    if center.nil? or oth.center.nil?
      false
    else
      ((center.first - oth.center.first).abs < @@radius) and ((center.last - oth.center.last).abs < @@radius)
    end
  end

  def self.reorder previous, current 

    previous.each_with_index do |p,i|
      next if p.center.nil?
      found = false
      current.each_with_index do |c,k|
        if p.compare c
          previous[i] = c
          current[k] = Entity.new        
          found = true
          break
        end
      end
      unless found
        previous[i] = Entity.new
      end
    end

    current.each do |c|
      next if c.center.nil?
      previous.each_with_index do |p,i|
        if p.center.nil?
          previous[i] = c 
          break
        end
      end
    end
  end

  def self.status
    status = []
    8.times do
      status << Entity.new
    end
    status
  end

end

