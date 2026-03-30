import { useState } from 'react'
import PlayTab from './PlayTab'
import AnalysisTab from './AnalysisTab'

type Tab = 'play' | 'analyze'

export default function App() {
  const [activeTab, setActiveTab] = useState<Tab>('play')

  return (
    <div className="min-h-screen bg-bg text-copy flex flex-col">
      <header className="flex items-center gap-4 px-8 py-4 border-b border-line bg-surface shrink-0">
        <span className="text-2xl leading-none">♟</span>
        <h1 className="text-lg font-bold tracking-wide">Bitcrusher</h1>

        <nav className="flex gap-1 mx-auto">
          {(['play', 'analyze'] as const).map(tab => (
            <button
              key={tab}
              onClick={() => setActiveTab(tab)}
              className={[
                'px-6 py-1.5 rounded-lg border text-sm font-semibold capitalize transition-all',
                activeTab === tab
                  ? 'bg-accent border-accent text-white'
                  : 'border-line text-muted hover:border-accent hover:text-copy',
              ].join(' ')}
            >
              {tab === 'play' ? 'Play' : 'Analyze'}
            </button>
          ))}
        </nav>
      </header>

      {activeTab === 'play' ? <PlayTab /> : <AnalysisTab />}
    </div>
  )
}
