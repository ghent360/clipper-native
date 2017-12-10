interface Point {
    x:number;
    y:number;
}

async function main() {
    let cl = await import("./clipper_js.js");
    let c = new cl.Clipper<Point>(1000);
    console.log('success');
}

main();