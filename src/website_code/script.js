// Initialize Icons
document.addEventListener('DOMContentLoaded', () => {
    lucide.createIcons();
    new TentacleAnimation('canvas');
});

/**
 * MODULE: Configuration
 * Central location for all tunable parameters.
 */
const AppConfig = {
    tentacles: {
        count: 50,
        segmentCount: 20,
        segmentLength: 15,
        baseRadius: 2,
        tipRadius: 1,
    },
    colors: {
        head: '#00ff9d',
        tail: '#bd00ff',
        bgClear: 'rgba(0,0,0,0)' 
    },
    physics: {
        friction: 0.1,
        gravity: 0.5,
        mouseInfluenceRadius: 800
    }
};

/**
 * MODULE: Segment
 * Represents a single bone/joint in a tentacle.
 */
class Segment {
    constructor(parent, length, angle) {
        this.parent = parent; 
        this.l = length;
        this.angle = angle;
        this.targetAngle = angle;
        this.x = 0;
        this.y = 0;
        
        // Initialize position if parent exists
        if(parent.x !== undefined) this.update(); 
    }

    update() {
        // Find anchor point (parent's end or parent's position)
        let px = this.parent.x;
        let py = this.parent.y;
        
        if (this.parent instanceof Segment) {
            px = this.parent.getEndX();
            py = this.parent.getEndY();
        }

        // Smoothly rotate towards target (Simple linear interpolation)
        this.angle += (this.targetAngle - this.angle) * AppConfig.physics.friction;

        this.x = px;
        this.y = py;
    }

    getEndX() {
        return this.x + Math.cos(this.angle) * this.l;
    }

    getEndY() {
        return this.y + Math.sin(this.angle) * this.l;
    }
}

/**
 * MODULE: Tentacle
 * A collection of Segments acting as a single limb.
 */
class Tentacle {
    constructor(x, y) {
        this.x = x;
        this.y = y;
        this.segments = [];
        // Randomized personality for each tentacle
        this.angleOffset = (Math.random() * 0.1) + 0.05;
        this.timeOffset = Math.random() * 1000;
        
        this.initSegments();
    }

    initSegments() {
        let prev = this;
        for (let i = 0; i < AppConfig.tentacles.segmentCount; i++) {
            // Taper the length slightly towards the tip for organic look
            let len = AppConfig.tentacles.segmentLength * (1 - (i / AppConfig.tentacles.segmentCount) * 0.5);
            // Start pointing DOWN (Math.PI / 2)
            let seg = new Segment(prev, len, Math.PI / 2);
            this.segments.push(seg);
            prev = seg;
        }
    }

    update(targetX, targetY, t) {
        let dx = targetX - this.x;
        let dy = targetY - this.y;
        let dist = Math.sqrt(dx*dx + dy*dy);
        let angleToMouse = Math.atan2(dy, dx);

        for (let i = 0; i < this.segments.length; i++) {
            let seg = this.segments[i];
            
            // 1. Calculate Base Movement (Currents/Waves)
            let wave = Math.sin(t * this.angleOffset + (i * 0.2) + this.timeOffset) * 0.1;
            
            // 2. Calculate Mouse Interaction
            // Stronger pull for segments closer to the tip (i / length)
            let tipInfluence = (i / this.segments.length); 
            
            // Mouse influence decreases with distance from tentacle base
            let mousePull = Math.max(0, 1 - (dist / AppConfig.physics.mouseInfluenceRadius)); 
            
            // 3. Determine Target Angle
            let restAngle = Math.PI / 2 + wave; // Default hanging down state
            
            // Blend between Rest State and Mouse Target based on proximity
            seg.targetAngle = (restAngle * (1 - mousePull)) + (angleToMouse * mousePull);
            
            // Add secondary ripple for liveness
            seg.targetAngle += Math.sin(t * 3 + i * 0.3) * 0.05;
            
            seg.update();
        }
    }

    draw(ctx) {
        ctx.beginPath();
        ctx.moveTo(this.x, this.y);
        
        for (let i = 0; i < this.segments.length; i++) {
            let seg = this.segments[i];
            ctx.lineTo(seg.getEndX(), seg.getEndY());
        }
        
        // Create gradient for this specific tentacle
        let lastSeg = this.segments[this.segments.length-1];
        let grad = ctx.createLinearGradient(this.x, this.y, lastSeg.getEndX(), lastSeg.getEndY());
        grad.addColorStop(0, AppConfig.colors.tail);
        grad.addColorStop(1, AppConfig.colors.head);
        
        ctx.strokeStyle = grad;
        ctx.lineWidth = 2;
        ctx.lineCap = 'round';
        ctx.stroke();

        // Draw Tip
        ctx.beginPath();
        ctx.fillStyle = AppConfig.colors.head;
        ctx.arc(lastSeg.getEndX(), lastSeg.getEndY(), AppConfig.tentacles.baseRadius, 0, Math.PI * 2);
        ctx.fill();
    }
}

/**
 * MODULE: Animation Engine
 * Handles the Canvas context, resize events, and the main loop.
 */
class TentacleAnimation {
    constructor(canvasId) {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext('2d');
        this.header = document.querySelector('header');
        
        this.width = 0;
        this.height = 0;
        this.tentacles = [];
        this.time = 0;
        this.mouse = { x: window.innerWidth / 2, y: window.innerHeight / 2 };

        this.bindEvents();
        this.resize();
        this.animate = this.animate.bind(this); // Fix 'this' context
        this.animate();
    }

    bindEvents() {
        window.addEventListener('resize', () => this.resize());
        window.addEventListener('mousemove', (e) => {
            this.mouse.x = e.clientX;
            this.mouse.y = e.clientY;
        });
        window.addEventListener('touchmove', (e) => {
            this.mouse.x = e.touches[0].clientX;
            this.mouse.y = e.touches[0].clientY;
        });
    }

    resize() {
        // Resize canvas to match the Header container
        this.width = this.canvas.width = this.header.clientWidth;
        this.height = this.canvas.height = this.header.clientHeight;
        this.initTentacles();
    }

    initTentacles() {
        this.tentacles = [];
        let gap = this.width / AppConfig.tentacles.count;
        
        // Create tentacles spanning wider than the screen for coverage
        for (let i = 0; i < AppConfig.tentacles.count + 10; i++) {
            let x = (i * gap) - (gap * 5); 
            let y = -20; // Start just above the viewport
            this.tentacles.push(new Tentacle(x, y));
        }
    }

    animate() {
        this.ctx.clearRect(0, 0, this.width, this.height);
        this.time += 0.01;

        this.tentacles.forEach(tentacle => {
            tentacle.update(this.mouse.x, this.mouse.y, this.time);
            tentacle.draw(this.ctx);
        });

        requestAnimationFrame(this.animate);
    }
}

fetch("README.md")
    .then(res => res.text())
    .then(md => {
        document.getElementById("readme-content").innerHTML = marked.parse(md);
    })
    .catch(err => {
        document.getElementById("readme-content").innerHTML = "Failed to load README.";
        console.error(err);
    });

