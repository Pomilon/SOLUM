// SOLUM Playground App

let solumModule = null;

// Initialize SOLUM WASM Module
SolumModule().then(module => {
    solumModule = module;
    console.log("SOLUM WASM Initialized");
    updateOutput();
});

const inputEl = document.getElementById('input');
const previewEl = document.getElementById('preview');
const htmlOutputEl = document.getElementById('html-output');
const textOutputEl = document.getElementById('text-output');
const astOutputEl = document.getElementById('ast-output');
const tabBtns = document.querySelectorAll('.tab-btn');
const tabContents = document.querySelectorAll('.tab-content');

// Tab Switching Logic
tabBtns.forEach(btn => {
    btn.addEventListener('click', () => {
        const tab = btn.dataset.tab;
        
        tabBtns.forEach(b => b.classList.remove('active'));
        tabContents.forEach(c => c.classList.remove('active'));
        
        btn.classList.add('active');
        document.getElementById(`${tab}${tab === 'preview' ? '' : '-output'}`).classList.add('active');
    });
});

// Real-time Update Logic
inputEl.addEventListener('input', () => {
    if (solumModule) {
        updateOutput();
    }
});

function updateOutput() {
    const text = inputEl.value;
    const inputPtr = allocateString(text);
    
    // 1. Render HTML for Preview
    const htmlPtr = solumModule._solum_render(inputPtr);
    const html = solumModule.UTF8ToString(htmlPtr);
    previewEl.innerHTML = html;
    htmlOutputEl.textContent = html;
    
    // 2. Render JSON AST
    const jsonPtr = solumModule._solum_serialize(inputPtr);
    const json = solumModule.UTF8ToString(jsonPtr);
    try {
        const parsed = JSON.parse(json);
        astOutputEl.textContent = JSON.stringify(parsed, null, 2);
    } catch (e) {
        astOutputEl.textContent = json;
    }

    // 3. Plain Text
    const textPtr = solumModule._solum_render_text(inputPtr);
    textOutputEl.textContent = solumModule.UTF8ToString(textPtr);

    // Cleanup (optional in this context but good practice)
    solumModule._free(inputPtr);
}

// Helper to handle strings in Emscripten
function allocateString(str) {
    const ptr = solumModule._malloc(str.length * 4 + 1);
    solumModule.stringToUTF8(str, ptr, str.length * 4 + 1);
    return ptr;
}
